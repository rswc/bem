#include "taskNotifyMessage.h"

TaskNotifyMessage::TaskNotifyMessage() {}

BaseMessage::MessageType TaskNotifyMessage::GetType() const
{
    return TASK_NOTIFY;
}

BaseMessage::MessageBuffer TaskNotifyMessage::Serialize() const
{
    MessageBuffer buffer;
    ReserveHeader(buffer);

    buffer.Put<task_id_t>(task_id);
    buffer.Put<TaskStatus>(task_status);

    PutHeader(buffer);
    buffer.Seek(0);
    return buffer;
}

void TaskNotifyMessage::Deserialize(MessageBuffer& buffer)
{
    task_id = buffer.Get<task_id_t>();
    task_status = buffer.Get<TaskStatus>();
}
