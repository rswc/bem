#include "messageFactory.h"

#include "messages.h"


MessageFactory::MessageFactory()
{
    buf = std::make_unique<ByteBuffer>();
}

void MessageFactory::Fill(const char* data, size_t length)
{
    buf->PutN(data, length);

    if (messageSize == 0 && buf->Length() >= BaseMessage::HEADER_SIZE)
    {
        // Extract total message length from common message header
        
        // GetAt() does not affect buffer position
        messageSize = buf->GetAt<size_t>(1);
    }
    
    if (messageSize > 0 && buf->Length() >= messageSize)
    {
        // TODO: only interpret if no message is ready
        //       (currently, we discard that previous message here)
        Interpret();
    }
}

// Try to interpret internal buffer as message
void MessageFactory::Interpret()
{
    Type type = buf->GetAt<Type>(0);

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
            case BaseMessage::MessageType::PING:
                message = std::make_unique<PingMessage>();
                break;
            case BaseMessage::MessageType::PONG:
                message = std::make_unique<PongMessage>();
                break;
            case BaseMessage::MessageType::HELLO:
                message = std::make_unique<HelloMessage>();
                break;
            case BaseMessage::MessageType::READY:
                message = std::make_unique<ReadyMessage>();
                break;
        }
        message->Deserialize(*buf);
    }

    ready = true;
}

bool MessageFactory::IsReady() const
{
    return ready;
}

// Get latest message & reset factory state
std::unique_ptr<BaseMessage> MessageFactory::Get()
{
    if (!ready)
        return MakeInvalid();

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
    
    ready = false;

    return std::move(message);
}

inline std::unique_ptr<BaseMessage> MessageFactory::MakeInvalid()
{
    return std::move(std::make_unique<InvalidMessage>());
}
