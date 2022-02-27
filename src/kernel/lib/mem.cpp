#include <lib/mem.h>

void memcpy(void *destination, const void *source, uint64 size)
{
    uint64 i = 0;
    for (; i < size / 8; i++)
    {
        ((uint64 *)destination)[i] = ((uint64 *)source)[i];
    }

    for (i *= 8; i < size; i++)
    {
        ((uint8 *)destination)[i] = ((uint8 *)source)[i];
    }
}

bool memcmp(const void *ptr1, const void *ptr2, uint64 size)
{
    for (uint64 i = 0; i < size; i++)
    {
        if (((uint8 *)ptr1)[i] != ((uint8 *)ptr2)[i])
            return false;
    }
    return true;
}

void memset(const void *ptr, uint8 value, uint64 size)
{
    uint64 i = 0;
    if (value == 0)
    {
        for (; i < size / 8; i++)
        {
            ((uint64 *)ptr)[i] = 0;
        }

        for (i *= 8; i < size; i++)
        {
            ((uint8 *)ptr)[i] = 0;
        }
    }
    else
    {
        for (; i < size; i++)
        {
            ((uint8 *)ptr)[i] = value;
        }
    }
}