#pragma once

#include "baseMessage.h"

class ResultMessage : public BaseMessage
{
private:
    uint32_t task_id;
    
public:
    ResultMessage();
    void init(uint32_t task_id);
    
    virtual MessageType GetType() const;
    MessageBuffer Serialize() const;
    void Deserialize(MessageBuffer& buffer);
    
    uint32_t getId() { return task_id; }
};
