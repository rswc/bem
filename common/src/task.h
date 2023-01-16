#pragma once

#include <string>

#include "byteBuffer.h"
#include "gamelist.h"

using task_id_t = uint32_t; 

extern const task_id_t TASK_ID_NONE;
extern const task_id_t TASK_ID_FIRST;

enum TaskStatus : int8_t {
    TS_NONE = 0,
    TS_QUESTION = 1,
    TS_RUNNING = 2,
    TS_CANCELED = 3,
};

struct Task
{
    task_id_t id = 0;
    games_id_t game_id = 0;
    games_id_t agent1 = 0;
    games_id_t agent2 = 0;
    uint32_t move_limit_ms = 20000;
    uint32_t games = 1u;
    uint16_t board_size = 8u;
   
    void init(task_id_t id, games_id_t game_id, games_id_t agent1, games_id_t agent2, uint32_t move_limit_ms,  
        uint32_t games, uint16_t board_size) {
        this->id = id;
        this->game_id = game_id;
        this->agent1 = agent1;
        this->agent2 = agent2;
        this->move_limit_ms = move_limit_ms;
        this->games = games;
        this->board_size = board_size;
    }

    ByteBuffer Serialize(ByteBuffer& buffer) const;
    void Deserialize(ByteBuffer& buffer);
};
