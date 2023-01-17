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
#include "maintenance.h"

#include "messages.h"

#include "gamelist.h"
#include "json.hpp"
#include "config.h"


#include <unistd.h>

bool setup(State& state, const std::string& configpath) {
    return load_config_from_file(state.config, configpath);
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
    std::thread t_maintenance(doMaintenance, std::ref(state));
    
    // Temporary scuff interface
    std::string cmd, token;
    while (!state.shouldQuit) // placeholder: replace with epoll in threads?
    {
        std::cout << "> ";
        std::cin >> cmd;

        if (cmd == "notify") {
            int nid;
            std::cin >> nid;

            auto msg = std::make_unique<TaskNotifyMessage>();
            msg->task_id = TASK_ID_NONE;
            msg->task_status = TS_QUESTION;

            state.mtx_nodes.lock();
            if (state.nodeExists(nid) && state.nodes[nid]->is_registered()) {
                state.nodes[nid]->mark_request();
                state.nodes[nid]->Send(std::move(msg));
            } else {
                std::cout << "Node with such ID does not exist or is not registered." << std::endl;
            }
            state.mtx_nodes.unlock();
        }
        else if (cmd == "terminate") {
            int nid;
            std::cin >> nid;

            state.mtx_nodes.lock();
            if (state.nodeExists(nid)) {
                // TODO: add gracefuly killing threads, destroying resources
                // TODO: WR or RDWR? Maybe node can send result? 
                shutdown(state.nodes[nid]->socket, SHUT_WR);
                state.nodes.erase(nid);
            } else {
                std::cout << "Node with such ID does not exist." << std::endl;
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

            std::cin >> gid >> ag1 >> ag2 >> board_size >> move_limit_ms >> games;

            Task task;
            task.init(TASK_ID_NONE, gid, ag1, ag2, board_size, move_limit_ms, games);

            auto node_ids = get_eligible_nodes_for_task(state, task);

            std::cout << "Node ids for given task: [";
            for (auto node_id : node_ids) {
                std::cout << node_id << ", ";
            }
            std::cout << "]" << std::endl;
            
            if (node_ids.empty()) {
                std::cout << "[!] No eligible nodes found. Cannot send task." << std::endl;
                std::cout << "GameID: " << gid << std::endl;
                std::cout << "AG1: " << ag1 << std::endl;
                std::cout << "AG2: " << ag2 << std::endl;
            } else { 
                size_t n_nodes = node_ids.size();
                std::vector<uint32_t> n_games(n_nodes, games / n_nodes);
                // first one is ceil() because otherwise we lose one game 
                n_games[0] = (games + n_nodes - 1) / n_nodes;
                

                std::cout << "Splitting tasks to eligible nodes:" << std::endl;
                
                state.mtx_nodes.lock();
                state.mtx_tasks.lock();

                for (size_t i = 0; i < n_nodes; i++) {
                    // maybe add min split value?
                    if (n_games[i] == 0) continue;
                
                    std::cout << "[" << node_ids[i] << "] -> " << n_games[i] << std::endl;

                    Task tt = task;
                    tt.id = next_task_id++;
                    tt.games = n_games[i];

                    auto ttp = std::make_shared<Task>(task);

                    state.tasks.insert({ttp->id, ttp});
                    state.nodes[node_ids[i]]->AssignTask(ttp);
                }

                state.mtx_nodes.unlock();
                state.mtx_tasks.unlock();
            }
        }
        else if (cmd == "nodes")
        {
            state.mtx_nodes.lock();
            for (auto& [node_id, node] : state.nodes)
            {
                assert(node_id == node->id);
                std::cout << "[" << node_id << "] " << inet_ntoa(node->addr.sin_addr)
                    << ':' << ntohs(node->addr.sin_port) << " flags<" << (node->is_registered() ? 'R' : 'N')
                    << ((node->flags & NodeFlag::CONN_PROBLEMS) ? 'P' : '.')
                    << ((node->flags & NodeFlag::CONN_BROKEN) ? 'B' : '.') << ">\n";
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
    t_maintenance.join();
    // join all from state.threads

    return 0;
}
