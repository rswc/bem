#pragma once

#include <sstream>


class ByteBuffer
{
private:
    std::stringbuf internal;
public:
    int Length();

    template <typename T>
    void Put(const T& data)
    {
        internal.sputn(reinterpret_cast<const char*>(&data), sizeof(data));
    }

    template <typename T>
    T Get()
    {
        T tmp;

        internal.sgetn(reinterpret_cast<char*>(&tmp), sizeof(tmp));

        return tmp;
    }

    template <typename T>
    void Get(T& dest)
    {
        internal.sgetn(reinterpret_cast<char*>(&dest), sizeof(dest));
    }

    template <typename T>
    void Get(T& dest, int n)
    {
        internal.sgetn(reinterpret_cast<char*>(&dest), n);
    }
};
