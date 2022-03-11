#pragma once

class Spinlock
{
public:
    inline void lock()
    {
        while (!__sync_bool_compare_and_swap(&_lock, 0, 1))
            __asm__("pause");

        __sync_synchronize();
    }
    inline void unlock()
    {
        _lock = false;
        __sync_synchronize();
    }
    inline bool islock()
    {
        return _lock;
    }

    inline bool tryLock()
    {
        if (_lock)
            return false;
        lock();
        return true;
    }

private:
    bool _lock = false;
};
