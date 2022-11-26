#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <thread>

#include "accept.h"
#include "node.h"
#include "readNode.h"

void setReuseAddr(int sock){
	const int one = 1;
	int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	if(res) error(1,errno, "setsockopt failed");
}

void acceptConnections(int port, State& state)
{
    int nextNodeId = 0;

    // open entry socket
    int entrySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (entrySocket == -1)
    {
        error(1, errno, "socket failed");
    }

    setReuseAddr(entrySocket);
    
    sockaddr_in entryAddr {
        .sin_family=AF_INET,
        .sin_port=htons(port),
        .sin_addr={INADDR_ANY}
    };
    if (bind(entrySocket, (sockaddr*) &entryAddr, sizeof(entryAddr)))
    {
        error(1, errno, "bind failed");
    }
	
    if (listen(entrySocket, 1))
    {
        error(1, errno, "listen failed");
    }

    // accept and register nodes
    while (!state.shouldQuit)
    {
        Node newNode;

        auto nodeSocket = accept(entrySocket, (sockaddr*) &newNode.addr, &newNode.addrSize);
		if (nodeSocket == -1)
        {
            error(1, errno, "accept failed");
        }

        //TODO: Check if node is already registered
        
        // else
        newNode.id = nextNodeId++;
        
        state.mtx_nodes.lock();
        state.nodes.push_back(newNode);

        int nidx = state.nodes.size() - 1;

        // spawn thread
        std::thread t_rdnode(readNode, nodeSocket, std::ref(state.nodes[nidx]), std::ref(state));

        state.mtx_nodes.unlock();


        state.mtx_threads.lock();

        state.threads.push_back(std::move(t_rdnode));

        state.mtx_threads.unlock();
    }

    close(entrySocket);
}
