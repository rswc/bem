#include "result.h"


void serialize(const Result& result, ByteBuffer& buffer) {
    buffer.Put<uint32_t>(result.games);
    buffer.Put<uint32_t>(result.failed_games);
    buffer.Put<uint32_t>(result.win_agent1);
    buffer.Put<uint32_t>(result.win_agent2);
    buffer.Put<uint32_t>(result.timeout_agent1);
    buffer.Put<uint32_t>(result.timeout_agent2);
}

Result deserialize(ByteBuffer& buffer) {
    Result result;
    result.games = buffer.Get<uint32_t>();
    result.failed_games = buffer.Get<uint32_t>();
    result.win_agent1 = buffer.Get<uint32_t>();
    result.win_agent2 = buffer.Get<uint32_t>();
    result.timeout_agent1 = buffer.Get<uint32_t>();
    result.timeout_agent2 = buffer.Get<uint32_t>();
    return result;
}