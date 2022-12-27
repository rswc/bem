#include "helloMessage.h"

#include <cassert>

void HelloMessage::init(uint8_t protoVer = 0, uint8_t agents = 0) {
    this->protocolVersion = protoVer;
    this->agentsCount = agents;
}

HelloMessage::HelloMessage() {}

BaseMessage::MessageType HelloMessage::GetType() const
{
    return HELLO;
}

BaseMessage::MessageBuffer HelloMessage::Serialize() const
{
    MessageBuffer buf;
    PutHeader(buf);
    buf.Put<uint8_t>(this->protocolVersion);
    buf.Put<uint8_t>(this->agentsCount);
    PutHeader(buf);
    buf.Seek(0);
    return buf;
}

void HelloMessage::Deserialize(MessageBuffer& buffer)
{
    protocolVersion = buffer.Get<uint8_t>();
    agentsCount = buffer.Get<uint8_t>();
}
