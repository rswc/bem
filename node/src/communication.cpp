#include "communication.h"
#include "messageFactory.h"

#include <unistd.h>
#include <error.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>

const size_t MAX_NODE_READ_BUFFER_SIZE = 1024;

bool connect_to_server(State& state) {
	addrinfo *server, hints={.ai_flags=0, .ai_family=AF_INET, .ai_socktype=SOCK_STREAM};
    
    std::string host = state.config.host;
    std::string service = std::to_string(state.config.port);

    int res = getaddrinfo(host.c_str(), service.c_str(), &hints, &server); 
	if (res || !server) {
        error(1, 0, "getaddrinfo: %s", gai_strerror(res));
        return false;
    }

	int sock = socket(server->ai_family, server->ai_socktype, 0);
	if (sock == -1) {
        error(1, errno, "socket failed");
        return false;
    }

	res = connect(sock, server->ai_addr, server->ai_addrlen);
	if (res) {
        error(1, errno, "connect failed");
        return false;
    }

	freeaddrinfo(server);

    state.socket = sock;
    return true;
}

void read_from_server_in_loop(State& state, int socket, size_t read_size) {
    assert (read_size <= MAX_NODE_READ_BUFFER_SIZE);
    char buffer[MAX_NODE_READ_BUFFER_SIZE];
    MessageFactory factory;

    while (true) 
    {
        auto len = read(socket, &buffer, read_size);
	    if (len == -1) 
        {
            error(1, errno, "read failed on server");
        } 
        else if (len > 0)
        {
            factory.Fill(buffer, len);

            for (auto& msg : factory.readyMessages)
            {
                std::unique_lock<std::mutex> guard(state.mtx_recvQueue);
                state.recvMessageQueue.push_back(std::move(msg));
                guard.unlock();
                state.cv_recvQueue.notify_one();
            }

            factory.FinishExtraction();
        } 
        else {
            break;
        }
    }
}


void write_to_server_in_loop(State& state, int socket) {
    while (true) {
        std::unique_lock<std::mutex> guard(state.mtx_sendQueue);
        state.cv_sendQueue.wait(guard, [&]() { 
            return !state.sendMessageQueue.empty(); 
        });

        // take message object from queue
        auto msg = std::move(state.sendMessageQueue.front());
        state.sendMessageQueue.pop_front();
        guard.unlock();
        
        auto mbuf = msg->Serialize();
        auto ret = write(socket, mbuf.Next(), mbuf.RemainingBytes());

        if  (ret == -1) 
        {
            error(1, errno, "write failed");
        } 
        mbuf.Advance(ret);
    }
}
