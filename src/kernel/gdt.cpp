#include <gdt.h>
#include <scheduler.h>

namespace Gdt
{

    constexpr uint8 PRESENT = 1 << 7;
    constexpr uint8 NOT_SYS = 1 << 4;
    constexpr uint8 EXEC = 1 << 3;
    constexpr uint8 DC = 1 << 2;
    constexpr uint8 RW = 1 << 1;

    constexpr uint8 GRAN = 1 << 3;
    constexpr uint8 SIZE = 1 << 2;
    constexpr uint8 LONG = 1 << 1;

    void install()
    {
        SegmentDescriptor *gdt = (SegmentDescriptor *)kcalloc(3, sizeof(SegmentDescriptor));
        gdt[1].limitLow = 0xFFFF;
        gdt[1].accessByte = PRESENT | NOT_SYS | EXEC | DC | RW;
        gdt[1].flags = GRAN | LONG;
        gdt[1].limitHigh = 0xF;

        gdt[2].limitLow = 0xFFFF;
        gdt[2].accessByte = PRESENT | NOT_SYS | RW;
        gdt[2].flags = GRAN | SIZE;
        gdt[2].limitHigh = 0xF;

        Scheduler::CPU *cpu = Scheduler::getCurrentCPU();

        cpu->gdt.base = (uint64)gdt;
        cpu->gdt.limit = sizeof(SegmentDescriptor) * 3 - 1;
        __asm__ volatile("lgdt (%%rax)" ::"a"(&cpu->gdt));
    }

} // namespace Gdt
