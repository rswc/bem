#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "task.h"

enum NodeFlags {
    NONE,
    REGISTERED = 1
};

class Node
{
private:
    std::vector<std::shared_ptr<Task>> tasks;

public:
    int id = -1;
    sockaddr_in addr{0};
    socklen_t addrSize = sizeof(addr);
    int flags = NONE;

    // Message queue simulator
    char lastCh = ' ';
    bool sendMessage = false;
    std::mutex mtx_msgQueue;
    std::condition_variable cv_msgQueue;

    void assignTask(std::shared_ptr<Task> task);
};
