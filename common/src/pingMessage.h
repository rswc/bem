#pragma once

#include "baseMessage.h"

class PingMessage : public BaseMessage
{
private:
    
public:
    PingMessage();
    virtual MessageType GetType() const;
    MessageBuffer Serialize() const;
    void Deserialize(MessageBuffer& buffer);
};
