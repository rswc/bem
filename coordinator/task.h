#pragma once

#include <string>

#include "byteBuffer.h"

struct Task
{
    int id;
    std::string cmd;

    ByteBuffer Serialize(ByteBuffer& buffer) const;
    void Deserialize(ByteBuffer& buffer);

};
