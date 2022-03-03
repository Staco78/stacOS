#pragma once
#include <types.h>

inline uint8 inb(uint16 port)
{
    uint8 ret;
    __asm__ volatile("inb %1, %0"
                 : "=a"(ret)
                 : "Nd"(port));
    return ret;
}

inline uint16 inw(uint16 port)
{
    uint16 ret;
    __asm__ volatile("inw %1, %0"
                 : "=a"(ret)
                 : "Nd"(port));
    return ret;
}

inline uint32 ind(uint16 port)
{
    uint32 ret;
    __asm__ volatile("inl %1, %0"
                 : "=a"(ret)
                 : "Nd"(port));
    return ret;
}

inline void outb(uint16 port, uint8 value)
{
    __asm__ volatile("outb %0, %1" ::"a"(value), "Nd"(port));
}

inline void outw(uint16 port, uint16 value)
{
    __asm__ volatile("outw %0, %1" ::"a"(value), "Nd"(port));
}

inline void outd(uint16 port, uint32 value)
{
    __asm__ volatile("outl %0, %1" ::"a"(value), "Nd"(port));
}
