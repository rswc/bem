#include "writeNode.h"

#include <unistd.h>
#include <error.h>
#include <string>

void writeNode(int sock, std::shared_ptr<Node> node, State& state)
{
    while (!state.shouldQuit)
    {
        std::unique_lock<std::mutex> lck(node->mtx_msgQueue);
        node->cv_msgQueue.wait(lck, [&node]() { return node->sendMessage; });

        // take message object from queue
        node->sendMessage = false;
        lck.unlock();
        
        std::string msg = "sup man";
        int cur = 0;

        while (cur < msg.size() - 1)
        {
            auto ret = write(sock, msg.c_str() + cur, msg.size() - cur);
            if  (ret == -1)
            {
                error(1, errno, "write failed on node %d", node->id);
            }

            cur += ret;
        }
    }
}