#include <cpu.h>

namespace cpu
{
    void writeMSR(uint32 msr, uint64 value)
    {
        __asm__ volatile("wrmsr"
                         :
                         : "a"((uint32)value), "d"((uint32)(value >> 32)), "c"(msr));
    }

    uint64 readMSR(uint32 msr)
    {
        uint32 lo, hi;
        __asm__ volatile("rdmsr"
                         : "=a"(lo), "=d"(hi)
                         : "c"(msr));
        return lo | ((uint64)hi << 32);
    }
} // namespace cpu
