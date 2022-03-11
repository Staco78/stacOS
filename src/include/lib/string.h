#pragma once
#include <types.h>
#include <lib/vector.h>
#include <lib/mem.h>
#include <memory.h>

class String : public Vector<char>
{
private:
public:
    String() {}
    String(const char *str)
    {
        memcpy(begin(), str, strlen(str));
    }
};