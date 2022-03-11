#pragma once
#include <types.h>
#include <memory.h>

template <typename T>
class Vector
{
private:
    static constexpr uint64 defaultDataSize = 10;

    T *_data = nullptr;
    uint64 _size = 0;
    uint64 dataSize = 0;

public:
    Vector()
    {
        realloc(defaultDataSize);
    }

    Vector(uint64 dataSize)
    {
        realloc(dataSize);
    }

    ~Vector()
    {
        kfree(_data);
    }

    void realloc(uint64 newSize)
    {
        dataSize = newSize;
        if (_data == nullptr)
            _data = (T *)kmalloc(newSize * sizeof(T));
        else
            _data = (T *)krealloc(_data, newSize * sizeof(T));
    }

    void push(T data)
    {
        if (_size == dataSize)
        {
            realloc(dataSize ? dataSize * 2 : 1);
        }

        _data[_size++] = data;
    }

    inline T &operator[](uint64 index)
    {
        return _data[index];
    }

    inline const T &operator[](uint64 index) const
    {
        return _data[index];
    }

    inline bool empty() const
    {
        return _size == 0;
    }

    inline uint64 size() const
    {
        return _size;
    }

    inline T *begin()
    {
        return _data;
    }

    inline T *end()
    {
        return _data + _size;
    }
};