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

    while (true) 
    {
        auto len = read(sock, &buf, 32);
	    if (len == -1) 
        {
            error(1, errno, "read failed on server");
        }

        if (len > 0)
        {
            factory.Fill(buf, len);

            for (auto& msg : factory.readyMessages)
            {
                std::unique_lock<std::mutex> guard(state.mtx_recvQueue);
                state.recvMessageQueue.push_back(std::move(msg));
                guard.unlock();
                state.cv_recvQueue.notify_one();
            }

            factory.FinishExtraction();
        }
    }
}
