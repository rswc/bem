#pragma once

#include "byteBuffer.h"


class BaseMessage
{
public:
    enum MessageType : char {
        NONE,
        ERROR,
        HELLO,  // Server sends hello with its current config
        READY,  // Node sends sync proof
        SYNC,   // Node can request assets
        TASK,   // 
        RESULT,
        PING,   // Heartbeat from server
        PONG,   // Result
        N_MESSAGE_TYPES,
    };

    static const size_t HEADER_SIZE = sizeof(MessageType) + sizeof(size_t);
    
    static const size_t TYPE_OFFSET = 0;
    static const size_t SIZE_OFFSET = 1;

    virtual MessageType GetType() const = 0;

protected:
    typedef ByteBuffer MessageBuffer;

    void PutHeader(MessageBuffer& buffer) const;
    void ReserveHeader(MessageBuffer& buffer) const;
    
public:
    virtual MessageBuffer Serialize() const = 0;
    // TODO: it should be documented, that buffer does not contain type and length
    virtual void Deserialize(MessageBuffer& buffer) = 0;
    
};
