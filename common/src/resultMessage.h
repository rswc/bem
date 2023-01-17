#pragma once

#include "baseMessage.h"
#include "result.h"
#include "task.h"

class ResultMessage : public BaseMessage
{
private:
    task_id_t task_id = TASK_ID_NONE;
    Result result;
    
public:
    ResultMessage();
    void init(uint32_t task_id, const Result& result);
    
    virtual MessageType GetType() const;
    MessageBuffer Serialize() const;
    void Deserialize(MessageBuffer& buffer);
    
    task_id_t get_task_id() { return task_id; }
    Result get_result() { return result; }

};
