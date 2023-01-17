#include "resultMessage.h"

#include <cassert>

void ResultMessage::init(uint32_t task_id, const Result& result) {
    this->task_id = task_id;
    this->result = result;
}

ResultMessage::ResultMessage() {}

BaseMessage::MessageType ResultMessage::GetType() const
{
    return RESULT;
}

BaseMessage::MessageBuffer ResultMessage::Serialize() const
{
    MessageBuffer buffer;
    ReserveHeader(buffer);

    buffer.Put<task_id_t>(this->task_id);
    serialize(this->result, buffer);

    PutHeader(buffer);
    buffer.Seek(0);

    return buffer;
}

void ResultMessage::Deserialize(MessageBuffer& buffer)
{
    this->task_id = buffer.Get<task_id_t>();
    this->result = deserialize(buffer);
}
