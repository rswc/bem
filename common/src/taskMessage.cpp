#include "taskMessage.h"

BaseMessage::MessageType TaskMessage::GetType() const
{
    return TASK;
}

BaseMessage::MessageBuffer TaskMessage::Serialize() const
{
    MessageBuffer buf;

    // HACK?: Reserve space. This should work
    // so long as header size is independent
    // of the message body. I think.
    PutHeader(buf);

    task.Serialize(buf);

    // Fill in actual header values
    PutHeader(buf);

    // Message contents are now serialized
    // Reset position to 0, to prepare for writing
    buf.Seek(0);

    return buf;
}

void TaskMessage::Deserialize(MessageBuffer& buffer)
{
    task.Deserialize(buffer);
}
