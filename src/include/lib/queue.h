#pragma once
#include <types.h>
#include <memory.h>
#include <debug.h>

template <typename T>
class Queue
{
private:
    static constexpr uint64 defaultDataSize = 10;

    T *_data = nullptr;
    uint64 _size = 0;
    uint64 dataSize = 0;
    uint64 lastRead = 0;
    uint64 lastWritten = 0;

public:
    Queue()
    {
        dataSize = defaultDataSize;
        _data = (T *)kmalloc(sizeof(T) * dataSize);
    }

    Queue(uint64 defaultDataSize)
    {
        dataSize = defaultDataSize;
        _data = (T *)kmalloc(sizeof(T) * dataSize);
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
        if (_size >= dataSize)
        {
            realloc(dataSize ? dataSize * 2 : 1);
        }
        lastWritten = lastWritten % dataSize;
        _data[lastWritten] = data;
        lastWritten++;
        _size++;
    }

    T &front()
    {
        assert(_size > 0);
        return _data[lastRead];
    }

    T &pop()
    {
        T &data = front();
        lastRead = (lastRead + 1) % dataSize;
        _size--;
        return data;
    }

    inline uint64 size()
    {
        return _size;
    }

    inline bool empty()
    {
        return _size == 0;
    }
};