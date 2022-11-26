#include <vector>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <mutex>

#include "node.h"
#include "accept.h"
#include "state.h"


int main (int argc, char* argv[])
{
    // load config
    short port = 12345; //TODO: Move to config file

    State state;

    std::thread t_acc(acceptConnections, port, std::ref(state));

    // Temporary scuff interface
    std::string cmd, token;
    while (!state.shouldQuit) // placeholder: replace with epoll in threads?
    {
        std::cin >> cmd;

        if (cmd == "acc")
        {
            std::cin >> token;
            int nid = std::stoi(token);

            state.mtx_nodes.lock();
            for (auto& node : state.nodes)
            {
                if (node.id == nid)
                {
                    node.flags |= NodeFlags::REGISTERED;
                }
            }
            state.mtx_nodes.unlock();
        }
        else if (cmd == "list")
        {
            state.mtx_nodes.lock();
            for (auto& node : state.nodes)
            {
                std::cout << "[" << node.id << "] " << inet_ntoa(node.addr.sin_addr)
                    << ':' << ntohs(node.addr.sin_port) << " flag<" << node.flags << ">"
                    << " lch \"" << node.lastCh << "\"\n";
            }
            state.mtx_nodes.unlock();
        }
        else if (cmd == "quit")
        {
            state.shouldQuit = true;
        }
        else
        {
            std::cout << "Invalid command. \n";
        }
    }

    t_acc.join();
    // join all from state.threads

}
