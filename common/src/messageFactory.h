#pragma once

#include <memory>

#include "byteBuffer.h"
#include "baseMessage.h"


class MessageFactory
{
private:
    typedef BaseMessage::MessageType Type;

    std::unique_ptr<ByteBuffer> buf;
    size_t messageSize = 0;
    std::unique_ptr<BaseMessage> message;
    bool ready = false;

    void Interpret();
    inline std::unique_ptr<BaseMessage> MakeInvalid();
    
public:
    void Fill(const char* data, size_t length);
    bool IsReady() const;
    std::unique_ptr<BaseMessage> Get();

    MessageFactory();
};
