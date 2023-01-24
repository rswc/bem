#pragma once

#include "byteBuffer.h"

struct Result {
    size_t games;
    size_t win_agent1;
    size_t win_agent2;
};

void serialize(const Result& result, ByteBuffer& buffer);
Result deserialize(ByteBuffer& buffer);
