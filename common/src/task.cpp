#include "task.h"


ByteBuffer Task::Serialize(ByteBuffer& buffer) const
{
    buffer.Put(cmd.size());
    for (char ch : cmd)
    {
        buffer.Put(ch);
    }

    return buffer;
}

void Task::Deserialize(ByteBuffer& buffer)
{
    size_t len = buffer.Get<size_t>();

    cmd.resize(len);
    buffer.Get(&cmd[0], len);
}
