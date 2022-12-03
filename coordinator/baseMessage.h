#pragma once

#include "byteBuffer.h"


class BaseMessage
{
public:
    enum MessageType {
        NONE,
        ERROR,
        HELLO,
        TASK
    };

protected:
    typedef ByteBuffer MessageBuffer;
    
public:
    virtual MessageBuffer Serialize() const = 0;
    virtual void Deserialize(MessageBuffer& buffer) = 0;

    void WriteNextBytes();
};
