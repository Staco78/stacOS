#pragma once
#include <types.h>

inline void outb(uint16 port, uint16 value)
{
    __asm__ volatile("outb %%al,%%dx" ::"a"(value), "d"(port));
}

inline uint16 inb(uint16 port)
{
    uint16 _v;
    __asm__ volatile("inb %%dx,%%al"
                     : "=a"(_v)
                     : "d"(port));
    return _v;
}
