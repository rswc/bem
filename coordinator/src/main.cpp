#include <vector>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <mutex>

#include "node.h"
#include "accept.h"
#include "state.h"
#include "taskMessage.h"
#include "handler.h"

#include "pingMessage.h"
#include "helloMessage.h"

int main (int argc, char* argv[])
{
    // load config
    short port = 12345; //TODO: Move to config file

    State state;

    std::thread t_acc(acceptConnections, port, std::ref(state));
    std::thread t_handler(handleCoordinatorMessages, std::ref(state));
    
    // Temporary scuff interface
    std::string cmd, token;
    while (!state.shouldQuit) // placeholder: replace with epoll in threads?
    {
        std::cout << "> ";
        std::cin >> cmd;

        if (cmd == "hello")
        {
            std::cin >> token;
            int nid = std::stoi(token);

            auto msg = std::make_unique<HelloMessage>();
            msg->init(0, 0);

            state.mtx_nodes.lock();
            for (auto& node : state.nodes)
            {
                if (node->id == nid)
                {
                    node->Send(std::move(msg));
                    break;
                }
            }
            state.mtx_nodes.unlock();
        }
        else if (cmd == "ping")
        {
            std::cin >> token;
            int nid = std::stoi(token);
            auto msg = std::make_unique<PingMessage>();
            state.mtx_nodes.lock();
            for (auto& node : state.nodes)
            {
                if (node->id == nid && (node->flags & NodeFlags::REGISTERED) != 0)
                {
                    node->Send(std::move(msg));
                    break;
                }
            }
            state.mtx_nodes.unlock();
        }
        else if (cmd == "task")
        {
            std::cin >> token;
            int nid = std::stoi(token);

            auto msg =  std::make_unique<TaskMessage>();
            msg->task = Task(10, "Test Task");

            state.mtx_nodes.lock();
            for (auto& node : state.nodes)
            {
                if (node->id == nid && (node->flags & NodeFlags::REGISTERED) != 0)
                {
                    node->Send(std::move(msg));
                    break;
                }
            }
            state.mtx_nodes.unlock();
        }
        else if (cmd == "list")
        {
            state.mtx_nodes.lock();
            for (auto& node : state.nodes)
            {
                std::cout << "[" << node->id << "] " << inet_ntoa(node->addr.sin_addr)
                    << ':' << ntohs(node->addr.sin_port) << " flag<" << node->flags << ">\n";
            }
            state.mtx_nodes.unlock();
            std::cout.flush();
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
    t_handler.join();
    // join all from state.threads

}
