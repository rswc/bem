#pragma once

#include <mutex>
#include <vector>
#include <thread>
#include <memory>

#include "node.h"

class State
{
private:
    
public:
    std::vector<std::shared_ptr<Node>> nodes;
    std::mutex mtx_nodes;

    std::vector<std::thread> threads;
    std::mutex mtx_threads;

    std::deque<std::pair<int, std::unique_ptr<BaseMessage>>> recvMessageQueue;
    std::mutex mtx_recvQueue;
    std::condition_variable cv_recvQueue;
    
    bool shouldQuit = false;
};

