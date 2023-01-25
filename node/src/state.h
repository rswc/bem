#pragma once

#include <mutex>
#include <atomic>
#include <condition_variable>
#include <deque>

#include "baseMessage.h"
#include "task.h"
#include "config.h"

struct State { 
    std::atomic<int> socket;
    
    std::mutex mtx_terminate;
    
    NodeConfig config;
    std::mutex mtx_config;

    std::deque<std::unique_ptr<BaseMessage>> sendMessageQueue, recvMessageQueue;
    std::deque<Task> taskQueue;

    std::mutex mtx_sendQueue, mtx_recvQueue, mtx_taskQueue;
    std::condition_variable cv_sendQueue, cv_recvQueue, cv_taskQueue;
    
    std::atomic<task_id_t> current_task_id = TASK_ID_NONE;
    std::atomic<bool> should_quit = false;
};

State& getGlobalState();