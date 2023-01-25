#include "invalidMessage.h"


BaseMessage::MessageType InvalidMessage::GetType() const
{
    return NONE;
}

BaseMessage::MessageBuffer InvalidMessage::Serialize() const
{
    return BaseMessage::MessageBuffer();
}

void InvalidMessage::Deserialize(MessageBuffer& __attribute__((unused)) buffer)
{
    // Do nothing
}
