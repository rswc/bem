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

    void Interpret();
    inline std::unique_ptr<BaseMessage> MakeInvalid();
    
public:
    std::vector<std::unique_ptr<BaseMessage>> readyMessages;

    void Fill(const char* data, size_t length);

    inline void FinishExtraction()
    {
        readyMessages.clear();
    }

    MessageFactory();
};
