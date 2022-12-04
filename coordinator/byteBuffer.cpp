#include "byteBuffer.h"


int ByteBuffer::Length()
{
    return internal.size();
}

void ByteBuffer::Seek(int to)
{
    position = to;
}
