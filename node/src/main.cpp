#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <thread>

#include "state.h"
#include "write.h"
#include "read.h"
#include "taskMessage.h"


int main(int argc, char const *argv[])
{
    State state;

	addrinfo *server, hints={.ai_flags=0, .ai_family=AF_INET, .ai_socktype=SOCK_STREAM};
	
    // TODO: load from config
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

    std::thread t_write(writeServer, sock, std::ref(state));

    auto msg = std::make_unique<TaskMessage>();
    msg->task.cmd = "Hello, Mario.";

    state.Send(std::move(msg));

    while (true)
    {
        // do tasks
    }

    t_write.join();

    close(sock);

    return 0;
}

