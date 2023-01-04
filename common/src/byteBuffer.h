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
    inline size_t Length() const
    {
        return internal.size();
    }

    inline size_t RemainingBytes() const
    {
        return Length() - position;
    }

    inline void Seek(int to)
    {
        position = to;
    }

    inline void Advance(int by)
    {
        position += by;
    }

    inline const char* Next() const
    {
        return &internal[position];
    }

    template <typename T>
    void Put(const T& data)
    {
        if (position + sizeof(data) > internal.size())
            internal.resize(position + sizeof(data));
    
        memcpy(&internal[position], &data, sizeof(data));

        position += sizeof(data);
    }

    template <typename T>
    void PutN(const T* data, size_t n)
    {
        size_t tsize = n * sizeof(T);

        if (position + tsize > internal.size())
            internal.resize(position + tsize);
    
        memcpy(&internal[position], data, tsize);

        position += tsize;
    }

    template <typename T>
    T GetAt(size_t index)
    {
        if (index + sizeof(T) <= internal.size())
            return *reinterpret_cast<T*>(&internal[index]);
        
        throw std::invalid_argument("Buffer index out of range");
    }

    template <typename T>
    void GetAt(T& dest, size_t index)
    {
        if (index + sizeof(T) <= internal.size())
        {
            memcpy(&dest, &internal[index], sizeof(T));
            return;
        }

        throw std::invalid_argument("Buffer index out of range");
    }

    template <typename T>
    void GetAt(T* dest, size_t index, int n)
    {
        if (index + n - 1 <= internal.size())
        {
            memcpy(dest, &internal[index], n);
            return;
        }
        
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
