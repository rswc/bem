#include "taskMessage.h"

BaseMessage::MessageType TaskMessage::GetType() const
{
    return TASK;
}

BaseMessage::MessageBuffer TaskMessage::Serialize() const
{
    MessageBuffer buf;
    ReserveHeader(buf);

    task.Serialize(buf);

    PutHeader(buf);
    buf.Seek(0);
    return buf;
}

void TaskMessage::Deserialize(MessageBuffer& buffer)
{
    task.Deserialize(buffer);
}
