#include "taskMessage.h"


BaseMessage::MessageBuffer TaskMessage::Serialize() const
{
    MessageBuffer buf;

    buf.Put(task.cmd.size());
    for (char ch : task.cmd)
    {
        buf.Put(ch);
    }

    return buf;
}

void TaskMessage::Deserialize(MessageBuffer& buffer)
{
    size_t len = buffer.Get<size_t>();

    task.cmd.resize(len);
    buffer.Get(task.cmd[0], len);
}
