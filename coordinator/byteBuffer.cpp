#include "byteBuffer.h"


int ByteBuffer::Length() const
{
    return internal.size();
}

int ByteBuffer::RemainingBytes() const
{
    return Length() - position;
}

void ByteBuffer::Seek(int to)
{
    position = to;
}

void ByteBuffer::Advance(int by)
{
    position += by;
}

const char* ByteBuffer::Next() const
{
    return &internal[position];
}
