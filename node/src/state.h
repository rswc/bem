#pragma once

#include <mutex>
#include <condition_variable>
#include <deque>

#include "baseMessage.h"
#include "task.h"
#include "config.h"

struct State { 

    NodeConfig config;
    std::mutex mtx_sendQueue, mtx_recvQueue, mtx_taskQueue, mtx_config;
    std::condition_variable cv_sendQueue, cv_recvQueue, cv_taskQueue;

    std::deque<std::unique_ptr<BaseMessage>> sendMessageQueue, recvMessageQueue;
    std::deque<Task> taskQueue;

    void Send(std::unique_ptr<BaseMessage> message);
    std::unique_ptr<BaseMessage> Receive();
};
