#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <thread>
#include <memory>
#include <iostream>

#include "accept.h"
#include "node.h"
#include "readNode.h"
#include "writeNode.h"

void setReuseAddr(int sock){
	const int one = 1;
	int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	if(res) error(1,errno, "setsockopt failed");
}

void acceptConnections(int port, State& state)
{
    node_id_t next_node_id = NODE_ID_FIRST;

    // open entry socket
    int entrySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (entrySocket == -1)
    {
        error(1, errno, "socket failed");
    }

    setReuseAddr(entrySocket);
    
    sockaddr_in entryAddr;
    std::memset(&entryAddr, 0, sizeof(sockaddr_in));
    entryAddr.sin_family = AF_INET;
    entryAddr.sin_port = htons(port);
    entryAddr.sin_addr = { INADDR_ANY };

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
        auto newNode = std::make_shared<Node>();

        auto nodeSocket = accept(entrySocket, (sockaddr*) &newNode->addr, &newNode->addrSize);
		if (nodeSocket == -1)
        {
            error(1, errno, "accept failed");
        }

        //TODO: Switch from using cout to maybe other log
        if (state.nodeExists(next_node_id)) {
            std::cout << "Node with ID " << next_node_id << " is already registered." << std::endl;
            continue;
        } 

        // TODO: make sure not to move below with Node mutex active?
        // If balancer at the same moment is trying to load
        state.mtx_taskBalancing.lock();
        state.suspectedBalancing = true;
        state.mtx_taskBalancing.unlock();

        newNode->socket = nodeSocket;
        node_id_t node_id = newNode->id = next_node_id++;

        state.mtx_nodes.lock();
        state.nodes.insert({node_id, std::move(newNode)});
        std::thread t_rdnode(readNode, nodeSocket, state.nodes[node_id], std::ref(state));
        std::thread t_wrnode(writeNode, nodeSocket, state.nodes[node_id], std::ref(state));
        state.mtx_nodes.unlock();


        state.mtx_threads.lock();
        state.threads.push_back(std::move(t_rdnode));
        state.threads.push_back(std::move(t_wrnode));
        state.mtx_threads.unlock();
    }

    close(entrySocket);
}
