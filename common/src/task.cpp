#include "task.h"

const task_id_t TASK_ID_NONE = 0u;
const task_id_t TASK_ID_FIRST = 1u;

void Task::Serialize(ByteBuffer& buffer) const
{
    buffer.Put<task_id_t>(id);
    buffer.Put<games_id_t>(game_id);
    buffer.Put<games_id_t>(agent1);
    buffer.Put<games_id_t>(agent2);
    buffer.Put<uint32_t>(board_size);
    buffer.Put<uint32_t>(move_limit_ms); 
    buffer.Put<uint32_t>(games);
}

void Task::Deserialize(ByteBuffer& buffer)
{
    id = buffer.Get<task_id_t>();
    game_id = buffer.Get<games_id_t>();
    agent1 = buffer.Get<games_id_t>();
    agent2 = buffer.Get<games_id_t>();
    board_size = buffer.Get<uint32_t>();
    move_limit_ms = buffer.Get<uint32_t>(); 
    games = buffer.Get<uint32_t>();
}
