#pragma once

#include <mutex>
#include <condition_variable>
#include <deque>

#include "baseMessage.h"


class State
{
public:
    std::mutex mtx_msgQueue;
    std::condition_variable cv_msgQueue;
    std::deque<std::unique_ptr<BaseMessage>> messageQueue;

    void Send(std::unique_ptr<BaseMessage> message);

};
