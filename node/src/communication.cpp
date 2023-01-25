#include "communication.h"
#include "messageFactory.h"
#include "graceful.h" // terminate_program

#include <unistd.h>
#include <error.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>

#include <string>

const size_t MAX_NODE_READ_BUFFER_SIZE = 1024;

bool connect_to_server() {
	addrinfo *server, hints; 

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    std::string host = getGlobalState().config.host;
    std::string service = std::to_string(getGlobalState().config.port);

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

    getGlobalState().socket = sock;
    return true;
}

void read_from_server_in_loop(int socket, size_t read_size) {
    assert (read_size <= MAX_NODE_READ_BUFFER_SIZE);
    char buffer[MAX_NODE_READ_BUFFER_SIZE];
    MessageFactory factory;

    while (!getGlobalState().should_quit) {

        auto len = read(socket, &buffer, read_size);
	    if (len == -1) {
            error(1, errno, "read failed on server");
        } 
        else if (len > 0) {
            factory.Fill(buffer, len);

            for (auto& msg : factory.readyMessages) {
                std::unique_lock<std::mutex> guard(getGlobalState().mtx_recvQueue);
                getGlobalState().recvMessageQueue.push_back(std::move(msg));
                guard.unlock();
                getGlobalState().cv_recvQueue.notify_one();
            }

            factory.FinishExtraction();
        } 
        else {
            break;
        }
    }
    std::cerr << "[RFS]: Read from server loop broken. Returning." << std::endl;
    terminate_program();
}


void write_to_server_in_loop(int socket) {
    while (!getGlobalState().should_quit) {
        std::unique_lock<std::mutex> guard(getGlobalState().mtx_sendQueue);
        getGlobalState().cv_sendQueue.wait(guard, [&]() { 
            return getGlobalState().should_quit || !getGlobalState().sendMessageQueue.empty(); 
        });
        
        if (getGlobalState().should_quit) break;

        // take message object from queue
        auto msg = std::move(getGlobalState().sendMessageQueue.front());
        getGlobalState().sendMessageQueue.pop_front();
        guard.unlock();
        
        auto mbuf = msg->Serialize();
        auto ret = write(socket, mbuf.Next(), mbuf.RemainingBytes());

        if  (ret == -1) { 
            // TODO: should we interpret other codes? like EAGAIN
            error(1, errno, "write failed");
        } else if (ret == 0) {
            // connection was closed
            break;
        }
        mbuf.Advance(ret);
    }
    std::cerr << "[WTS]: Write to server loop broken. Returning." << std::endl;
    terminate_program();
}
