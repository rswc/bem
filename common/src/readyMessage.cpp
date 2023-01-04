#include "readyMessage.h"

#include <cassert>

ReadyMessage::ReadyMessage() {}

BaseMessage::MessageType ReadyMessage::GetType() const
{
    return READY;
}

BaseMessage::MessageBuffer ReadyMessage::Serialize() const
{
    MessageBuffer buf;
    ReserveHeader(buf);
    PutHeader(buf);
    buf.Seek(0);
    return buf;
}

void ReadyMessage::Deserialize(MessageBuffer& buffer)
{
}
