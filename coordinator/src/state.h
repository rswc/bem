#pragma once

#include <mutex>
#include <vector>
#include <thread>
#include <memory>
#include <unordered_map>
#include <atomic>

#include "node.h"
#include "task.h"
#include "config.h"

struct State 
{
    CoordinatorConfig config;
    std::mutex mtx_config;

    std::unordered_map<node_id_t, std::shared_ptr<Node>> nodes;
    std::mutex mtx_nodes;

    std::unordered_map<task_id_t, std::shared_ptr<TaskGroup>> groups;
    std::mutex mtx_groups;

    std::unordered_map<task_id_t, std::shared_ptr<Task>> tasks;
    std::mutex mtx_tasks;

    std::vector<std::thread> threads;
    std::mutex mtx_threads;

    std::deque<std::pair<node_id_t, std::unique_ptr<BaseMessage>>> recvMessageQueue;
    std::mutex mtx_recvQueue;
    std::condition_variable cv_recvQueue;
    
    std::atomic<bool> shouldQuit = false;
    std::atomic<bool> suspectedBalancing = false;
    std::mutex mtx_taskBalancing;

    bool nodeExists(int node_id) { return nodes.find(node_id) != nodes.end(); }
    bool groupExists(int group_id) { return groups.find(group_id) != groups.end(); }
    
    void terminateNode(node_id_t nid)
    {
        mtx_nodes.lock();
        
        if (!nodeExists(nid)) {
            std::cerr << "[TN]: Node with id = " << nid << " does not exist" << std::endl;
            mtx_nodes.unlock();
            return;
        }

        // TODO: add gracefuly killing threads, destroying resources
        // TODO: WR or RDWR? Maybe node can send result? 
        shutdown(nodes[nid]->socket, SHUT_WR);
        nodes.erase(nid);

        mtx_nodes.unlock();
        
        balanceTasks(*this);
    }
};

