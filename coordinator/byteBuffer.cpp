#include "byteBuffer.h"


int ByteBuffer::Length()
{
    return internal.in_avail();
}
