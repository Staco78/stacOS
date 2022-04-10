#pragma once
#include <types.h>
#include <memory.h>
#include <operators.h>
#include <lib/mem.h>

template <typename T>
class Vector
{
protected:
    static constexpr uint64 defaultDataSize = 10;

    mutable T *_data = nullptr;
    mutable uint64 _size = 0;
    mutable uint64 dataSize = 0;

public:
    Vector()
    {
        realloc(defaultDataSize);
    }

    Vector(uint64 dataSize)
    {
        realloc(dataSize);
    }

    Vector(const Vector<T> &other)
    {
        realloc(other.dataSize);
        memcpy(begin(), other.begin(), other.size() * sizeof(T));
        _size = other._size;
    }

    inline void operator=(const Vector<T> &other)
    {
        realloc(other.size());
        _size = other.size();
        memcpy(begin(), other.begin(), other.size() * sizeof(T));
    }

    ~Vector()
    {
        kfree(_data);
    }

    void realloc(uint64 newSize) const
    {
        dataSize = newSize;
        if (_data == nullptr)
            _data = (T *)kmalloc(newSize * sizeof(T));
        else
            _data = (T *)krealloc(_data, newSize * sizeof(T));
    }

    void push(const T &data)
    {
        if (_size == dataSize)
        {
            realloc(dataSize ? dataSize * 2 : 1);
        }

        T &ref = _data[_size++];
        new (&ref) T(data);
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

    inline const T *begin() const
    {
        return _data;
    }

    inline T *end()
    {
        return _data + _size;
    }

    template <class Function>
    T *find(Function func)
    {
        for (uint i = 0; i < _size; i++)
        {
            if (func(_data[i]))
                return &_data[i];
        }
        return nullptr;
    }
};