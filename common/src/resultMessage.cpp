#include "resultMessage.h"

#include <cassert>

void ResultMessage::init(uint32_t task_id) {
    this->task_id = task_id;
}

ResultMessage::ResultMessage() {}

BaseMessage::MessageType ResultMessage::GetType() const
{
    return RESULT;
}

BaseMessage::MessageBuffer ResultMessage::Serialize() const
{
    MessageBuffer buf;
    PutHeader(buf);
    buf.Put<uint32_t>(this->task_id);
    PutHeader(buf);
    buf.Seek(0);
    return buf;
}

void ResultMessage::Deserialize(MessageBuffer& buffer)
{
    this->task_id = buffer.Get<uint32_t>();
}
