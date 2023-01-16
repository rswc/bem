#pragma once

#include "baseMessage.h"
#include "task.h"

class TaskNotifyMessage : public BaseMessage
{
public:

    TaskNotifyMessage();
    task_id_t task_id;
    TaskStatus task_status;
    virtual MessageType GetType() const;
    MessageBuffer Serialize() const;
    void Deserialize(MessageBuffer& buffer);
};
