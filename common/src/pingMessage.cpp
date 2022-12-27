#include "pingMessage.h"
#include <cassert>

PingMessage::PingMessage() {}

BaseMessage::MessageType PingMessage::GetType() const
{
    return PING;
}

BaseMessage::MessageBuffer PingMessage::Serialize() const
{
    MessageBuffer buf;
    PutHeader(buf);
    buf.Seek(0);
    return buf;
}

void PingMessage::Deserialize(MessageBuffer& buffer)
{
}
