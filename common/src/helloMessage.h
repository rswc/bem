#pragma once

#include "baseMessage.h"

class HelloMessage : public BaseMessage
{
private:
    uint8_t protocolVersion = 0;
    uint8_t agentsCount = 0;
    
public:
    HelloMessage();
    void init(uint8_t protoVer, uint8_t isSynced);

    virtual MessageType GetType() const;
    MessageBuffer Serialize() const;
    void Deserialize(MessageBuffer& buffer);
    
    int getProtocolVersion() { return static_cast<int>(protocolVersion); }
    int getAgentsCount() { return static_cast<int>(agentsCount); }
};
