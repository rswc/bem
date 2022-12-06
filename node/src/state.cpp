#include "state.h"


void State::Send(std::unique_ptr<BaseMessage> message)
{
    std::unique_lock<std::mutex> lck(mtx_msgQueue);
    
    messageQueue.push_back(std::move(message));

    if (messageQueue.size() == 1)
    {
        cv_msgQueue.notify_one();
    }
}
