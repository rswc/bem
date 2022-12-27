#pragma once

#include "baseMessage.h"

class PongMessage : public BaseMessage
{
private:
    
public:
    PongMessage();
    virtual MessageType GetType() const;
    MessageBuffer Serialize() const;
    void Deserialize(MessageBuffer& buffer);
};
