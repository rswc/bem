#include "invalidMessage.h"


BaseMessage::MessageType InvalidMessage::GetType() const
{
    return NONE;
}

BaseMessage::MessageBuffer InvalidMessage::Serialize() const
{
    return BaseMessage::MessageBuffer();
}

void InvalidMessage::Deserialize(__attribute__((unused)) MessageBuffer& buffer)
{
    // Do nothing
}
