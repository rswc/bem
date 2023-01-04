#include "pongMessage.h"
#include <cassert>

PongMessage::PongMessage() {}

BaseMessage::MessageType PongMessage::GetType() const
{
    return PONG;
}

BaseMessage::MessageBuffer PongMessage::Serialize() const
{
    MessageBuffer buf;
    ReserveHeader(buf);
    PutHeader(buf);
    buf.Seek(0);
    return buf;
}

void PongMessage::Deserialize(MessageBuffer& buffer)
{
}
