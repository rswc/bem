#include "state.h"


void State::Send(std::unique_ptr<BaseMessage> message)
{
    std::unique_lock<std::mutex> guard(mtx_sendQueue);
    sendMessageQueue.push_back(std::move(message));
    guard.unlock();
    cv_sendQueue.notify_one();
}

std::unique_ptr<BaseMessage> State::Receive()
{
    std::unique_lock<std::mutex> guard(mtx_recvQueue);
    cv_recvQueue.wait(guard, [&]() { 
        return !recvMessageQueue.empty(); 
    });

    auto msg = std::move(recvMessageQueue.front());
    recvMessageQueue.pop_front();
    return std::move(msg);
}
