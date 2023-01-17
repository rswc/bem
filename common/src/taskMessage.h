#pragma once

#include "baseMessage.h"
#include "task.h"


class TaskMessage : public BaseMessage
{
private:
public:
    virtual MessageType GetType() const;
    // TODO: adpapt to Task

    Task task;
    MessageBuffer Serialize() const;
    void Deserialize(MessageBuffer& buffer);
};
