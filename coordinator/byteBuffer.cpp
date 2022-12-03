#include "byteBuffer.h"


template <typename T>
void ByteBuffer::Put(const T& data)
{
    internal.sputn(reinterpret_cast<char*>(&data), sizeof(data));
}

template <typename T>
T ByteBuffer::Get()
{
    T tmp;

    internal.sgetn(reinterpret_cast<char*>(&tmp), sizeof(tmp));

    return tmp;
}

int ByteBuffer::Length()
{
    return internal.in_avail();
}
