#include "baseMessage.h"


void BaseMessage::PutHeader(MessageBuffer& buffer) const
{
    buffer.Seek(0);
    buffer.Put(GetType());
    buffer.Put(buffer.Length());
}

void BaseMessage::ReserveHeader(MessageBuffer& buffer) const
{
    // HACK?: Reserve space. This should work
    // so long as header size is independent
    // of the message body. I think.
    PutHeader(buffer);
}
