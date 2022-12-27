#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <thread>
#include <iostream>
#include <chrono>

#include "state.h"
#include "write.h"
#include "read.h"
#include "messages.h"

// TODO: load from config / add as arg params 
int connectToServer() {
	addrinfo *server, hints={.ai_flags=0, .ai_family=AF_INET, .ai_socktype=SOCK_STREAM};
	
    int res = getaddrinfo("localhost", "12345", &hints, &server); 
	if (res || !server)
    {
        error(1, 0, "getaddrinfo: %s", gai_strerror(res));
    }

	// create socket
	int sock = socket(server->ai_family, server->ai_socktype, 0);
	if (sock == -1)
    {
        error(1, errno, "socket failed");
    }

	// attept to connect
	res = connect(sock, server->ai_addr, server->ai_addrlen);
	if (res)
    {
        error(1, errno, "connect failed");
    }

	// free memory
	freeaddrinfo(server);

    return sock;
}

// TODO: read protocol version from config file
bool syncWithServer(State &state) {
    // protocol version 0, not synced (?)
    std::cout << "Waiting for HELLO message from the server..." << std::endl;

    auto msg = state.Receive();
    
    if (msg->GetType() != BaseMessage::HELLO) {
        std::cout << "Server returned message but its wrong." << std::endl;
        return false;
    }

    HelloMessage *msg_ptr = dynamic_cast<HelloMessage*>(msg.get());
    
    if (msg_ptr->getProtocolVersion() == 0) {
        std::cout << "Server returned correct protocol version." << std::endl;
        return true;
    } else {
        std::cout << "Sync with server failed" << std::endl;
        return false;
    }
    
    auto ack_msg = std::make_unique<ReadyMessage>(); 
    state.Send(std::move(ack_msg));
    return true;
}


void doTasks(State &state) {
    while (true) {
        std::unique_lock<std::mutex> guard(state.mtx_taskQueue);
        state.cv_taskQueue.wait(guard, [&]{
            return !state.taskQueue.empty();
        });
        Task task = state.taskQueue.front();
        state.taskQueue.pop_front();
        guard.unlock();
        
        std::cout << "[DT]: Task with id " << task.id << ", received. Sleeping for 10 seconds" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "[DT]: Task with id " << task.id << ", done. Sending result" << std::endl;
        
        auto result_msg = std::make_unique<ResultMessage>();
        result_msg->init(task.id);
        state.Send(std::move(result_msg));
    }
}

int main(int argc, char const *argv[])
{
    State state;

    int sock = connectToServer();

    // graceful exit?
    std::thread t_write(writeServer, sock, std::ref(state));
    std::thread t_read(readServer, sock, std::ref(state));
    std::thread t_task(doTasks, std::ref(state));

    bool result = syncWithServer(state);
    if (!result) {
        return 1;
    }

    while (true) {
        std::cout << "Waiting for server instructions..." << std::endl;
        auto msg = state.Receive();
        
        if (msg->GetType() == BaseMessage::PING) {
            // TODO: check for contents? 
            
            auto resp = std::make_unique<PongMessage>();
            std::cout << "Received PING. Sending PONG." << std::endl;
            state.Send(std::move(resp));
        }
        else if (msg->GetType() == BaseMessage::TASK) {
            TaskMessage *msg_ptr = dynamic_cast<TaskMessage*>(msg.get());
            Task task = msg_ptr->task;
            std::cout << "Received TASK. Appending to TaskQueue." << std::endl;

            std::unique_lock<std::mutex> guard(state.mtx_taskQueue);
            state.taskQueue.emplace_back(task);
            guard.unlock();
            state.cv_taskQueue.notify_one();
        }
    }

    t_write.join();
    t_read.join();
    t_task.join();
    close(sock);

    return 0;
}

