#pragma once

#include <string>

#include "byteBuffer.h"

struct Task
{
    int id;
    std::string cmd;
    
    Task(int id = 0, std::string cmd = "") : id(id), cmd(cmd) {}
    ByteBuffer Serialize(ByteBuffer& buffer) const;
    void Deserialize(ByteBuffer& buffer);

};
