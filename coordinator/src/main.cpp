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

#include "gamelist.h"
#include "json.hpp"
#include "config.h"

bool setup(State& state, const std::string& configpath) {
    return load_config_from_file(state.config, configpath);
}

std::vector<node_id_t> get_eligible_nodes_for_task(State& state, const Task& task) {
    std::vector<node_id_t> eligible_ids;
    state.mtx_nodes.lock();
    for (const auto&[node_id, node] : state.nodes) {
        if (!node->is_registered()) continue;
        if (!node->gamelist.contains_game(task.game_id)) continue;
        
        if (node->gamelist.contains_agent(task.game_id, task.agent1) 
            && node->gamelist.contains_agent(task.game_id, task.agent2)) {
        
            eligible_ids.push_back(node_id);
        }
        
    }
    state.mtx_nodes.unlock();
    return eligible_ids;
}

int main (int argc, char* argv[]) {

    // TODO: load config path from argv

    State state;
    std::string configpath = "coordinator.json";
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

        if (cmd == "ping")
        {
            std::cin >> token;
            int nid = std::stoi(token);
            auto msg = std::make_unique<PingMessage>();
            state.mtx_nodes.lock();
            if (state.nodeExists(nid)) {
                if (state.nodes[nid]->is_registered()) {
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
            // TODO: track tasks, make possible to cancel one 
            static task_id_t next_task_id = TASK_ID_FIRST;
            games_id_t gid, ag1, ag2;
            uint16_t board_size;
            uint32_t move_limit_ms, games;

            std::cin >> gid >> ag1 >> ag2 >> move_limit_ms >> board_size >> games;

            Task task;
            task.init(TASK_ID_NONE, gid, ag1, ag2, move_limit_ms, board_size, games);
            auto node_ids = get_eligible_nodes_for_task(state, task);

            std::cout << "Node ids for given task: [";
            for (auto node_id : node_ids) {
                std::cout << node_id << ", ";
            }
            std::cout << "]" << std::endl;
            
            if (node_ids.empty()) {
                std::cout << "[!] No eligible nodes found. Cannot send task.";
            } else { 
                size_t n_nodes = node_ids.size();
                std::vector<uint32_t> n_games(n_nodes, games / n_nodes);
                // first one is ceil() because otherwise we loose one game 
                n_games[0] = (games + n_nodes - 1) / n_nodes;
                

                std::cout << "Splitting tasks to eligible nodes:" << std::endl;
                state.mtx_nodes.lock();
                for (size_t i = 0; i < n_nodes; i++) {
                    std::cout << "[" << node_ids[i] << "] -> " << n_games[i] << std::endl;
                    auto msg = std::make_unique<TaskMessage>();
                    task.id = next_task_id++;
                    msg->task = task;
                    state.nodes[node_ids[i]]->Send(std::move(msg));
                }
                state.mtx_nodes.unlock();
            }
        }
        else if (cmd == "nodes")
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
        else if (cmd == "games") {
            state.mtx_config.lock();
            state.config.gamelist.print();
            std::cout.flush();
            state.mtx_config.unlock();
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
