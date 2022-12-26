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
    switch (type)
    {
    case Type::TASK:
        message = std::make_unique<TaskMessage>();

        // yeah, alright
        message->Deserialize(*buf);
        break;
    
    default:
        message = MakeInvalid();
        break;
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
