#pragma once

#include "byteBuffer.h"

struct Result {
    uint32_t games;
    uint32_t failed_games; // number of games that returned exception, (not counted as win)
    uint32_t win_agent1;
    uint32_t win_agent2;
    uint32_t timeout_agent1; // number of round player2 has been timed out
    uint32_t timeout_agent2; // implies other player has won the game (included won games)

    Result merge(Result& other) {
        return Result (
            this->games + other.games,
            this->failed_games + other.failed_games,
            this->win_agent1 + other.win_agent1,
            this->win_agent2 + other.win_agent2,
            this->timeout_agent1 + other.timeout_agent1,
            this->timeout_agent2 + other.timeout_agent2
        );
    }

    Result(uint32_t games = 0, uint32_t failed_games = 0, uint32_t win_agent1 = 0, uint32_t win_agent2 = 0, uint32_t timeout1 = 0, uint32_t timeout2 = 0) 
        : games(games), failed_games(failed_games), win_agent1(win_agent1), win_agent2(win_agent2), timeout_agent1(timeout1), timeout_agent2(timeout2) {};
};

void serialize(const Result& result, ByteBuffer& buffer);
Result deserialize(ByteBuffer& buffer);
