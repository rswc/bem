#pragma once

#include "byteBuffer.h"

struct Result {
    size_t games;
    size_t win_agent1;
    size_t win_agent2;

    Result merge(Result& other) {
        return Result (
            this->games + other.games,
            this->win_agent1 + other.win_agent1,
            this->win_agent2 + other.win_agent2
        );
    }

    Result(size_t games = 0, size_t win_agent1 = 0, size_t win_agent2 = 0) : games(games), win_agent1(win_agent1), win_agent2(win_agent2) {};
};

void serialize(const Result& result, ByteBuffer& buffer);
Result deserialize(ByteBuffer& buffer);
