#include "readNode.h"

#include <unistd.h>
#include <error.h>
#include <iostream>

#include "messageFactory.h"

void readNode(int sock, std::shared_ptr<Node> node, State& state)
{
    char buf[32];
    MessageFactory factory;

    while (!state.shouldQuit || !node->shouldQuit)
    {
        auto len = read(sock, &buf, 32);
	    if (len == -1) 
        {
            error(1, errno, "read failed on node %d", node->id);
        } else if (len == 0) {
            std::cout << "[READN]: NodeQuit, connection dropped." << std::endl;
            node->shouldQuit = true;
            break;
        }

        if (len > 0)
        {
            factory.Fill(buf, len);

            if (factory.IsReady())
            {
                auto msg = factory.Get();
                std::unique_lock<std::mutex> guard(state.mtx_recvQueue);
                state.recvMessageQueue.emplace_back(node->id, std::move(msg));
                guard.unlock();
                state.cv_recvQueue.notify_one();
            }
        }
    }
}
