#include "helloMessage.h"

#include <cassert>

void HelloMessage::init(uint8_t protocol_version, char flag, const GameList& gamelist) {
    m_proto_ver = protocol_version;
    m_flag = flag;
    m_gamelist = gamelist;
}

HelloMessage::HelloMessage() {}

BaseMessage::MessageType HelloMessage::GetType() const
{
    return HELLO;
}

BaseMessage::MessageBuffer HelloMessage::Serialize() const
{
    MessageBuffer buf;
    ReserveHeader(buf);

    buf.Put<uint8_t>(m_proto_ver);
    buf.Put<char>(m_flag);
    
    json j {{ "gamelist", this->m_gamelist }};
    std::vector<uint8_t> serialized_games = json::to_bson(j);
    
    for (uint8_t byte : serialized_games) {
        buf.Put<uint8_t>(byte);
    }
    PutHeader(buf);
    buf.Seek(0);
    return buf;
}

void HelloMessage::Deserialize(MessageBuffer& buffer)
{
    m_proto_ver = buffer.Get<uint8_t>();
    m_flag = buffer.Get<char>();

    // TODO: optimize? instead of copying byte by byte
    std::vector<uint8_t> bytes;
    while (buffer.RemainingBytes()) {
        bytes.push_back(buffer.Get<uint8_t>());
    }
    
    json j = json::from_bson(bytes);
    j.at("gamelist").get_to(m_gamelist);
}
