#include "taskMessage.h"


BaseMessage::MessageBuffer TaskMessage::Serialize() const
{
    MessageBuffer buf;

    task.Serialize(buf);

    // message contents are now serialized
    // reset position to 0, to prepare for writing
    buf.Seek(0);

    return buf;
}

void TaskMessage::Deserialize(MessageBuffer& buffer)
{
    task.Deserialize(buffer);
}
