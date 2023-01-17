#include "result.h"

void serialize(const Result& result, ByteBuffer& buffer) {
    buffer.Put<size_t>(result.games);
    buffer.Put<size_t>(result.win_agent1);
    buffer.Put<size_t>(result.win_agent2);
}

Result deserialize(ByteBuffer& buffer) {
    Result result;
    result.games = buffer.Get<size_t>();
    result.win_agent1 = buffer.Get<size_t>();
    result.win_agent2 = buffer.Get<size_t>();
    return result;
}