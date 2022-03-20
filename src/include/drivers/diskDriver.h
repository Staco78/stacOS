#pragma once
#include <types.h>
#include <lib/mem.h>
#include <debug.h>

class DiskDriver
{
public:
    virtual int64 read(uint64 offset, uint64 size, void *buffer) = 0;
    virtual int64 write(uint64 offset, uint64 size, void *buffer) = 0;

    virtual bool canWrite() = 0;
    virtual uint64 getSize() = 0;
};

class InitrdDriver : public DiskDriver
{
private:
    uint64 m_size;

public:
    InitrdDriver(void *address, uint64 size)
    {
        buffer = (uint8 *)address;
        m_size = size;
    }

    uint64 getSize()
    {
        return m_size;
    }

    bool canWrite()
    {
        return false;
    }

    int64 read(uint64 offset, uint64 size, void *buffer) { assert(false); }
    int64 write(uint64 offset, uint64 size, void *buffer) { assert(false); }

    uint8 *buffer = nullptr;
};