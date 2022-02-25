#pragma once

#define assert(expr)                                             \
    if (!(expr))                                                 \
    {                                                            \
        _panic("Assert failed (" #expr ")", __LINE__, __FILE__); \
    }


#define BOCHS_BREAKPOINT() __asm__ volatile ("xchgw %bx, %bx")