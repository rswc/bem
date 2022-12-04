#pragma once

#include <vector>
#include <stdexcept>
#include <string.h>


class ByteBuffer
{
private:
    std::vector<char> internal;
    int position = 0;

public:
    int Length() const;
    int RemainingBytes() const;
    void Seek(int to);
    void Advance(int by);
    const char* Next() const;

    template <typename T>
    void Put(const T& data)
    {
        std::copy(reinterpret_cast<const char*>(&data), reinterpret_cast<const char*>(&data) + sizeof(data), std::back_inserter(internal));

        position += sizeof(data);
    }

    template <typename T>
    T GetAt(int index)
    {
        if (index + sizeof(T) <= internal.size())
            return *reinterpret_cast<T*>(&internal[index]);
        
        throw std::invalid_argument("Buffer index out of range");
    }

    template <typename T>
    void GetAt(T& dest, int index)
    {
        if (index + sizeof(T) <= internal.size())
            memcpy(&dest, &internal[index], sizeof(T));
            return;

        throw std::invalid_argument("Buffer index out of range");
    }

    template <typename T>
    void GetAt(T* dest, int index, int n)
    {
        if (index + n - 1 <= internal.size())
            memcpy(dest, &internal[index], n);
            return;
        
        throw std::invalid_argument("Buffer index out of range");
    }
    
    template <typename T>
    T Get()
    {
        T tmp = GetAt<T>(position);
        position += sizeof(T);
        return tmp;
    }

    template <typename T>
    void Get(T& dest)
    {
        GetAt(dest, position);
        position += sizeof(T);
    }

    template <typename T>
    void Get(T* dest, int n)
    {
        GetAt(dest, position, n);
        position += n;
    }
};
