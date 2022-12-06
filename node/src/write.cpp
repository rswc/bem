#include "write.h"

#include <unistd.h>
#include <error.h>
#include <string>


void writeServer(int sock, State& state)
{
    while (true)
    {
        std::unique_lock<std::mutex> lck(state.mtx_msgQueue);
        state.cv_msgQueue.wait(lck, [&]() { return state.messageQueue.size() > 0; });

        // take message object from queue
        auto msg = std::move(state.messageQueue.front());
        state.messageQueue.pop_front();

        lck.unlock();
        
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
