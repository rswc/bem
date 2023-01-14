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
#include "config.h"

int connectToServer(std::string host, short port) {
	addrinfo *server, hints={.ai_flags=0, .ai_family=AF_INET, .ai_socktype=SOCK_STREAM};
	
    std::string service = std::to_string(port);
    int res = getaddrinfo(host.c_str(), service.c_str(), &hints, &server); 
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

bool register_to_coordinator(State &state) {
    std::cout << "Sending HELLO message to coordinator..." << std::endl;
    state.mtx_config.lock();
    auto hello_msg = std::make_unique<HelloMessage>();
    hello_msg->init(state.config.protocol_version, HelloMessage::HelloFlag::NONE, state.config.gamelist);
    state.Send(std::move(hello_msg));
    state.mtx_config.unlock();

    bool sync_complete = false;
    while (!sync_complete) {
        std::cout << "Waiting for HELLO message from the coordinator..." << std::endl;

        auto msg = state.Receive();
        if (msg->GetType() != BaseMessage::HELLO) {
            std::cout << "[!] Server returned message but its wrong." << std::endl;
            return false;
        }

        HelloMessage *msg_ptr = dynamic_cast<HelloMessage*>(msg.get());
        switch (msg_ptr->flag()) {
            case HelloMessage::HelloFlag::REJECT: {
                std::cerr << "[!] Server returned REJECT flag in hello response." << std::endl;
                return false;
            } break;
            case HelloMessage::HelloFlag::ACCEPT: {
                std::cerr << "Server returned ACCEPT flag in hello response." << std::endl;
                sync_complete = true;
            } break;
            case HelloMessage::HelloFlag::SYNC: {
                std::cerr << "Server returned SYNC flag in hello response." << std::endl;
                sync_complete = true;
            } break;
            default: {
                std::cerr << "[!] Server returned unknown flag in hello response." << std::endl;
                return false;
            } break;
        }
    }
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

bool setup(State& state, const std::string& configpath) {
    // no mutex needed, we dont assume multihtreading here yet
    return load_config_from_file(configpath, state.config);
}

int main(int argc, char const *argv[]) {
    // TODO: load config from argv
    std::string configpath = "config.json";

    State state;
    bool setup_ok = setup(state, configpath);
    if (!setup_ok) {
        return 1;
    }

    int sock = connectToServer(state.config.host, state.config.port);

    // graceful exit?
    std::thread t_write(writeServer, sock, std::ref(state));
    std::thread t_read(readServer, sock, std::ref(state));
    std::thread t_task(doTasks, std::ref(state));

    bool register_ok = register_to_coordinator(state);
    if (!register_ok) {
        // TODO: graceful exit?
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

