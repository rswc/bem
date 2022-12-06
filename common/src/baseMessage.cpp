#include "baseMessage.h"


void BaseMessage::PutHeader(MessageBuffer& buffer) const
{
    buffer.Seek(0);
    buffer.Put(GetType());
    buffer.Put(buffer.Length());
}
