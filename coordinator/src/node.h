#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <deque>

#include "task.h"
#include "baseMessage.h"


class State;

enum NodeFlags {
    NONE,
    REGISTERED = 1,
    SYNCED = 2,
};

class Node
{
    friend void writeNode(int sock, std::shared_ptr<Node> node, State& state);
    friend void readNode(int sock, std::shared_ptr<Node> node, State& state);
private:
    std::vector<std::shared_ptr<Task>> tasks;

    std::mutex mtx_msgQueue;
    std::condition_variable cv_msgQueue;

public:
    int id = -1;
    sockaddr_in addr{0};
    socklen_t addrSize = sizeof(addr);
    int flags = NONE;
    
    std::deque<std::unique_ptr<BaseMessage>> messageQueue;

    void AssignTask(std::shared_ptr<Task> task);
    void Send(std::unique_ptr<BaseMessage> message);
};
