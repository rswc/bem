#pragma once

#include "byteBuffer.h"


class BaseMessage
{
public:
    enum MessageType : char {
        NONE,
        ERROR,
        HELLO,
        TASK
    };

    virtual inline MessageType GetType() const = 0;

protected:
    typedef ByteBuffer MessageBuffer;

    void PutHeader(MessageBuffer& buffer) const;
    
public:
    virtual MessageBuffer Serialize() const = 0;
    virtual void Deserialize(MessageBuffer& buffer) = 0;
    
};
