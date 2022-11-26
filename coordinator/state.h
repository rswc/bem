#pragma once

#include <mutex>
#include <vector>
#include "node.h"

class State
{
private:
    
public:
    std::vector<Node> nodes;
    std::mutex mtx_nodes;
    bool shouldQuit = false;
    
};

