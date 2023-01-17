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
    TS_CANCELLED = 3,
    TS_DONE = 4
};

struct Task
{
    task_id_t id = TASK_ID_NONE;
    games_id_t game_id = GAME_ID_NONE;
    games_id_t agent1 = GAME_ID_NONE;
    games_id_t agent2 = GAME_ID_NONE;
    uint32_t board_size = 8u;
    uint32_t move_limit_ms = 20000;
    uint32_t games = 1u;
    TaskStatus status = TS_NONE;
   
    void init(task_id_t id, games_id_t game_id, games_id_t agent1, games_id_t agent2, uint32_t board_size, uint32_t move_limit_ms,  
        uint32_t games) {
        this->id = id;
        this->game_id = game_id;
        this->agent1 = agent1;
        this->agent2 = agent2;
        this->board_size = board_size;
        this->move_limit_ms = move_limit_ms;
        this->games = games;
    }

    void Serialize(ByteBuffer& buffer) const;
    void Deserialize(ByteBuffer& buffer);
};
