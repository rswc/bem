#include "write.h"

#include <unistd.h>
#include <error.h>
#include <string>

void writeServer(int sock, State& state)
{
    while (true) {
        std::unique_lock<std::mutex> guard(state.mtx_sendQueue);
        state.cv_sendQueue.wait(guard, [&]() { 
            return !state.sendMessageQueue.empty(); 
        });

        // take message object from queue
        auto msg = std::move(state.sendMessageQueue.front());
        state.sendMessageQueue.pop_front();
        guard.unlock();
        
        auto mbuf = msg->Serialize();
        while (mbuf.RemainingBytes() > 0)
        {
            auto ret = write(sock, mbuf.Next(), mbuf.RemainingBytes());
            if  (ret == -1)
            {
                error(1, errno, "write failed");
            }

            mbuf.Advance(ret);
        }
    }
}
