#include "read.h"

#include <unistd.h>
#include <error.h>
#include <iostream>

#include "messageFactory.h"

void readServer(int sock, State& state)
{
    // TODO: remove magic number -> store in variable
    char buf[32];
    MessageFactory factory;

    while (!state.shouldQuit) 
    {
        auto len = read(sock, &buf, 32);
	    if (len == -1) 
        {
            error(1, errno, "read failed on server");
        } else if (len == 0) { 
            state.shouldQuit = true;
            break;
        }

        if (len > 0)
        {
            factory.Fill(buf, len);

            if (factory.IsReady())
            {
                auto msg = factory.Get();
                std::unique_lock<std::mutex> guard(state.mtx_recvQueue);
                state.recvMessageQueue.push_back(std::move(msg));
                guard.unlock();
                state.cv_recvQueue.notify_one();
            }
        }
    }
}
