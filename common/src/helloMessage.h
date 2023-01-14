#pragma once

#include "baseMessage.h"
#include "gamelist.h" 

class HelloMessage : public BaseMessage
{
private:
    uint8_t m_proto_ver = 0;
    char m_flag = 0;
    GameList m_gamelist;
public:
    
    enum HelloFlag : char {
        NONE = 0,
        REJECT = 0,
        ACCEPT = 1,
        SYNC = 2,
        REMOVE = 3,
    };

    HelloMessage();
    void init(uint8_t protocol_version, char flag, const GameList& gamelist);

    virtual MessageType GetType() const;
    MessageBuffer Serialize() const;
    void Deserialize(MessageBuffer& buffer);
    
    int protocol_version() { return static_cast<int>(m_proto_ver); }
    char flag() { return m_flag; }
    const GameList& gamelist() { return m_gamelist; }
};
