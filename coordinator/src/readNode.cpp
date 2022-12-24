#include "readNode.h"

#include <unistd.h>
#include <error.h>
#include <iostream>

#include "messageFactory.h"
#include "taskMessage.h"


void readNode(int sock, std::shared_ptr<Node> node, State& state)
{
    char buf[32];
    MessageFactory factory;

    while (!state.shouldQuit)
    {
        auto len = read(sock, &buf, 32);
	    if (len == -1) 
        {
            error(1, errno, "read failed on node %d", node->id);
        }

        if (len > 0)
        {
            factory.Fill(buf, len);

            if (factory.IsReady())
            {
                auto msg = factory.Get();

                // TODO: Pass to actual handler

                if (msg->GetType() == BaseMessage::TASK)
                {
                    auto tmsg = static_cast<TaskMessage*>(msg.get());
                    std::cout << "Task message received!\nTask: " << tmsg->task.cmd << '\n';
                }
            }
        }
    }
}
