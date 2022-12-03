#pragma once

#include <sstream>


class ByteBuffer
{
private:
    std::stringbuf internal;
public:
    template <typename T>
    void Put(const T& data);

    template <typename T>
    T Get();

    int Length();
};
