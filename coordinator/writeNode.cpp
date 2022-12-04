#include "writeNode.h"

#include <unistd.h>
#include <error.h>
#include <string>

void writeNode(int sock, std::shared_ptr<Node> node, State& state)
{
    while (!state.shouldQuit)
    {
        std::unique_lock<std::mutex> lck(node->mtx_msgQueue);
        node->cv_msgQueue.wait(lck, [&]() { return node->messageQueue.size() > 0; });

        // take message object from queue
        auto msg = std::move(node->messageQueue.front());
        node->messageQueue.pop_front();

        lck.unlock();
        
        auto mbuf = msg->Serialize();

        while (mbuf.RemainingBytes() > 0)
        {
            auto ret = write(sock, mbuf.Next(), mbuf.RemainingBytes());
            if  (ret == -1)
            {
                error(1, errno, "write failed on node %d", node->id);
            }

            mbuf.Advance(ret);
        }
    }
}