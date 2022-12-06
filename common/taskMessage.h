#pragma once

#include "baseMessage.h"
#include "task.h"


class TaskMessage : public BaseMessage
{
private:

public:
    Task task;
    MessageBuffer Serialize() const;
    void Deserialize(MessageBuffer& buffer);

};