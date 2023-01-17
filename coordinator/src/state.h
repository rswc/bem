#pragma once

#include <mutex>
#include <vector>
#include <thread>
#include <memory>
#include <unordered_map>

#include "node.h"
#include "task.h"
#include "config.h"

struct State 
{
    CoordinatorConfig config;
    std::mutex mtx_config;

    std::unordered_map<node_id_t, std::shared_ptr<Node>> nodes;
    std::mutex mtx_nodes;

    std::unordered_map<task_id_t, std::shared_ptr<Task>> tasks;
    std::mutex mtx_tasks;

    std::vector<std::thread> threads;
    std::mutex mtx_threads;

    std::deque<std::pair<node_id_t, std::unique_ptr<BaseMessage>>> recvMessageQueue;
    std::mutex mtx_recvQueue;
    std::condition_variable cv_recvQueue;
    
    bool shouldQuit = false;
    bool nodeExists(int node_id) { return nodes.find(node_id) != nodes.end(); }
};

