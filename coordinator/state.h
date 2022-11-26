#pragma once

#include <mutex>
#include <vector>
#include <thread>

#include "node.h"

class State
{
private:
    
public:
    std::vector<Node> nodes;
    std::mutex mtx_nodes;
    
    std::vector<std::thread> threads;
    std::mutex mtx_threads;
    
    bool shouldQuit = false;
};

