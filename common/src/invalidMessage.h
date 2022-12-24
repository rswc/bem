#pragma once

#include "baseMessage.h"


class InvalidMessage : public BaseMessage
{
private:

public:
    virtual MessageType GetType() const;

    MessageBuffer Serialize() const;
    void Deserialize(MessageBuffer& buffer);

};
