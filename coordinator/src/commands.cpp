#include "commands.h"

#include "node.h"
#include "accept.h"
#include "state.h"
#include "maintenance.h"

#include "messages.h"
#include "config.h"

#include <vector>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <mutex>
#include <cassert>

void command_help() {
    std::cout << "List of available commands:\n"
        << " - help - Show this list\n"
        << " - terminate <NODE_ID> - Terminate connection to node with the specified id\n"
        << " - nodes - List all nodes in this system\n"
        << " - tasks - List all tasks in this system\n"
        << " - games - List all games recognized by this system\n"
        << " - cancel <TASK_ID> - Cancel task with the specified id\n"
        << " - task <GAME_ID> <AGENT_1> <AGENT_2> <BOARD_SIZE> <MOVE_LIMIT_MS> <NUM_GAMES>\n"
        << "     - Create new task. NUM_GAMES battles of game identified by GAME_ID\n"
        << "       between AGENT_1 and AGENT_2 will be performed" << std::endl;
}

void command_terminate(State& state) {
    int nid;
    std::cin >> nid;
    state.terminateNode(nid);
}

void command_tasks(State& state) {
    state.mtx_groups.lock();

    for (auto& [group_id, group] : state.groups)
    {
        assert(group_id == group->id);
        std::cout << "[" << group_id << "] ";

        switch (group->status) {
            case TS_RUNNING: {
                std::cout << "RUNNING (waiting for " << group->remaining_tasks << " sub-tasks to complete)\n";
            } break;
            case TS_DONE: {
                std::cout << "DONE: " << group->aggregate_result << std::endl; 
            } break;
            case TS_CANCELLED: {
                std::cout << "CANCELLED\n";
            } break;
            default: {
                std::cout << "\n";
            }
        }

    }

    state.mtx_groups.unlock();
    std::cout.flush();
}

void command_games(State& state) {
    state.mtx_config.lock();
    state.config.gamelist.print();
    std::cout.flush();
    state.mtx_config.unlock();
}

void command_cancel(State& state) {
    task_id_t group_id;
    std::cin >> group_id;

    state.mtx_nodes.lock();
    state.mtx_tasks.lock();
    state.mtx_groups.lock();

    if (state.groupExists(group_id) && state.groups[group_id]->status == TS_RUNNING) {
        state.groups[group_id]->status = TS_CANCELLED;
        
        for (auto& [task_id, task] : state.tasks)
        {
            if (task->group_id != group_id) continue;
            if (task->status != TS_RUNNING) continue;

            for (auto& [node_id, node] : state.nodes)
            {
                node->UnassignTask(task_id);
            }
        }
    } else {
        std::cout << "Task with such ID does not exist or is not running." << std::endl;
    }

    state.mtx_nodes.unlock();
    state.mtx_tasks.unlock();
    state.mtx_groups.unlock();
}

void command_nodes(State& state) {
    state.mtx_nodes.lock();
    for (auto& [node_id, node] : state.nodes)
    {
        assert(node_id == node->id);
        std::cout << "[" << node_id << "] " << inet_ntoa(node->addr.sin_addr)
            << ':' << ntohs(node->addr.sin_port) << " flags<" << (node->is_registered() ? 'R' : 'N')
            << ((node->flags & NodeFlag::CONN_BROKEN) ? 'B' : '.') << ">";
        
        if (node->is_idle()) {
            std::cout << " IDLE";
        
        } else {
            std::cout << " RUNNING TASK [" << node->activeTaskGroup << "]";

        }

        std::cout << '\n';
    }
    state.mtx_nodes.unlock();
    std::cout.flush();
}

void command_task(State& state) {
    // TODO: track tasks, make possible to cancel one 
    static task_id_t next_task_id = TASK_ID_FIRST, next_group_id = TASK_ID_FIRST;
    games_id_t gid, ag1, ag2;
    uint32_t board_size, move_limit_ms, games;

    std::cin >> gid >> ag1 >> ag2 >> board_size >> move_limit_ms >> games;
    
    state.mtx_config.lock();
    bool valid_gid = state.config.gamelist.contains_game(gid);
    bool valid_ag1 = state.config.gamelist.contains_agent(gid, ag1);
    bool valid_ag2 = state.config.gamelist.contains_agent(gid, ag2);
    state.mtx_config.unlock();
    
    if (!valid_gid) {
        std::cout << "Invalid GameID: " << gid << " does not exists. Check `games` command for available ids.\n";
        return;
    } else if (!valid_ag1) {
        std::cout << "Invalid Agent1ID: " << ag1 << " does not exists. Check `games` command for available ids.\n";
        return;
    } else if (!valid_ag2) {
        std::cout << "Invalid Agent2ID: " << ag2 << " does not exists. Check `games` command for available ids.\n";
        return;
    } else if (board_size == 0 || move_limit_ms == 0 || games == 0) {
        std::cout << "Invalid parameters: at least one of given values is 0. \n";
    }

    // "Template" task -- it will be split into sub-tasks and divided
    // among available nodes
    Task task;
    task.init(TASK_ID_NONE, TASK_ID_NONE, gid, ag1, ag2, board_size, move_limit_ms, games);

    state.mtx_nodes.lock();
    auto node_ids = get_eligible_nodes_for_task(state, task);
    state.mtx_nodes.unlock();

    std::cout << "Node ids for given task: [";
    for (auto node_id : node_ids) {
        std::cout << node_id << ", ";
    }
    std::cout << "]" << std::endl;
    
    if (node_ids.empty()) {
        std::cout << "[!] No eligible nodes found. Cannot send task." << std::endl;
    } else { 
        size_t n_nodes = node_ids.size();
        std::vector<uint32_t> n_games(n_nodes, games / n_nodes);
        // first one is ceil() because otherwise we lose one game 
        n_games[0] = (games + n_nodes - 1) / n_nodes;

        std::cout << "Splitting tasks to eligible nodes:" << std::endl;
        
        state.mtx_nodes.lock();
        state.mtx_tasks.lock();
        state.mtx_groups.lock();

        TaskGroup tgroup(next_group_id++);
        tgroup.remaining_tasks = n_nodes;
        tgroup.status = TS_RUNNING;
        state.groups.insert({tgroup.id, std::make_shared<TaskGroup>(tgroup)});

        task.group_id = tgroup.id;

        for (size_t i = 0; i < n_nodes; i++) {
            // maybe add min split value?
            if (n_games[i] == 0) continue;
        
            std::cout << "[" << node_ids[i] << "] -> " << n_games[i] << std::endl;

            Task tt = task;
            tt.id = next_task_id++;
            tt.games = n_games[i];
            tt.status = TS_RUNNING;

            auto ttp = std::make_shared<Task>(tt);

            state.tasks.insert({ttp->id, ttp});
            state.nodes[node_ids[i]]->AssignTask(ttp);
        }

        state.mtx_nodes.unlock();
        state.mtx_tasks.unlock();
        state.mtx_groups.unlock();
    }
}