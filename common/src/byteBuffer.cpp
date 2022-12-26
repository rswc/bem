#include "byteBuffer.h"


size_t ByteBuffer::Length() const
{
    return internal.size();
}

size_t ByteBuffer::RemainingBytes() const
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