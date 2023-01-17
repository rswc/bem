#include "messageFactory.h"

#include "messages.h"

#include <cassert>


MessageFactory::MessageFactory()
{
    buf = std::make_unique<ByteBuffer>();
}

void MessageFactory::Fill(const char* data, size_t length)
{
    buf->PutN(data, length);

    while (buf->Length() >= BaseMessage::HEADER_SIZE)
    {
        if (messageSize == 0)
        {
            // Extract total message length from common message header
            
            // GetAt() does not affect buffer position
            messageSize = buf->GetAt<size_t>(BaseMessage::SIZE_OFFSET);

            if (messageSize < BaseMessage::HEADER_SIZE)
                throw std::invalid_argument("Bad message received: Length is shorter than header size");
        }
        else if (buf->Length() >= messageSize)
        {
            Interpret();
        } 
        else 
        {
            break;
        }
    }
}

// Try to interpret internal buffer as message
void MessageFactory::Interpret()
{
    std::unique_ptr<BaseMessage> message;
    Type type = buf->GetAt<Type>(BaseMessage::TYPE_OFFSET);

    buf->Seek(BaseMessage::HEADER_SIZE);

    // Create Message based on type
    
    // TODO: move validation to other function
    if (type <= BaseMessage::NONE || type >= BaseMessage::N_MESSAGE_TYPES) {
        message = MakeInvalid();
    } else {
        switch (type)
        {
            case BaseMessage::MessageType::TASK:
                message = std::make_unique<TaskMessage>();
                break;
            case BaseMessage::MessageType::RESULT:
                message = std::make_unique<ResultMessage>();
                break;
            case BaseMessage::MessageType::HELLO:
                message = std::make_unique<HelloMessage>();
                break;
            case BaseMessage::MessageType::TASK_NOTIFY:
                message = std::make_unique<TaskNotifyMessage>();
                break;
            default: 
                assert(0 && "Not implemented");
        }

        message->Deserialize(*buf);
    }

    readyMessages.push_back(std::move(message));

    // Reset message size to unknown for next run
    messageSize = 0;

    // Copy any remaining bytes for next run
    auto newBuf = std::make_unique<ByteBuffer>();

    // TODO: put a method for copying remaining bytes
    //       into separate buffer in ByteBuffer?
    //       We can't really do that here without exposing
    //       ByteBuffer's internal data structure
    while (buf->RemainingBytes())
    {
        newBuf->Put(buf->Get<char>());
    }

    buf = std::move(newBuf);
}

inline std::unique_ptr<BaseMessage> MessageFactory::MakeInvalid()
{
    return std::make_unique<InvalidMessage>();
}
