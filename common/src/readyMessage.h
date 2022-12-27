#pragma once

#include "baseMessage.h"

class ReadyMessage : public BaseMessage
{
private:
    
public:
    ReadyMessage();
    virtual MessageType GetType() const;
    MessageBuffer Serialize() const;
    void Deserialize(MessageBuffer& buffer);
};
