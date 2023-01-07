#include <vector>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <mutex>
#include <cassert>

#include "node.h"
#include "accept.h"
#include "state.h"
#include "taskMessage.h"
#include "handler.h"

#include "pingMessage.h"
#include "helloMessage.h"

#include "game.h"
#include "json.hpp"
#include "config.h"

bool setup(State& state, std::string configpath) {
    return load_config_from_file(configpath, state.config);
}

int main (int argc, char* argv[]) {

    // TODO: load config path from argv
    std::string configpath = "config.json";

    State state;
    bool result = setup(state, configpath);
    
    if (!result) {
        std::cout << "[!] Cannot load config from " << configpath << std::endl;
        return 1;
    }

    std::thread t_acc(acceptConnections, state.config.port, std::ref(state));
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
            if (state.nodeExists(nid)) {
                state.nodes[nid]->Send(std::move(msg));
            } else {
                std::cout << "Node with such ID does not exist" << std::endl;
            }
            state.mtx_nodes.unlock();
        }
        else if (cmd == "ping")
        {
            std::cin >> token;
            int nid = std::stoi(token);
            auto msg = std::make_unique<PingMessage>();
            state.mtx_nodes.lock();
            if (state.nodeExists(nid)) {
                if (state.nodes[nid]->flags & NodeFlags::REGISTERED != 0) {
                    state.nodes[nid]->Send(std::move(msg));
                } else {
                    std::cout << "Node with ID " << nid << " is not registered. Send HELLO first." << std::endl;
                }
            } else {
                std::cout << "Node with such ID does not exist" << std::endl;
            }
            state.mtx_nodes.unlock();
        }
        else if (cmd == "task")
        {
            std::cin >> token;
            int nid = std::stoi(token);

            auto msg =  std::make_unique<TaskMessage>();
            msg->task = Task(10, "Test Task");

            // TODO: maybe Make function SendToNode or something like that,  
            // - lock mutex, send message if Node is in given state
            // think about sending unique ptr on BaseMesssage?
            // return false if msg was not sent
            state.mtx_nodes.lock();
            if (state.nodeExists(nid)) {
                if (state.nodes[nid]->flags & NodeFlags::REGISTERED != 0) {
                    state.nodes[nid]->Send(std::move(msg));
                } else {
                    std::cout << "Node with ID " << nid << " is not registered. Send HELLO first." << std::endl;
                }
            } else {
                std::cout << "Node with such ID does not exist" << std::endl;
            }
            state.mtx_nodes.unlock();
        }
        else if (cmd == "list")
        {
            state.mtx_nodes.lock();
            for (auto& [node_id, node] : state.nodes)
            {
                assert(node_id == node->id);
                std::cout << "[" << node_id << "] " << inet_ntoa(node->addr.sin_addr)
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

    return 0;
}
