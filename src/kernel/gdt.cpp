#include <gdt.h>
#include <scheduler.h>
#include <lib/mem.h>

namespace gdt
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
        SegmentDescriptor *gdt = (SegmentDescriptor *)kmalloc(6 * sizeof(SegmentDescriptor) + sizeof(LongSegmentDescriptor));
        memset(gdt, 0, 6 * sizeof(SegmentDescriptor) + sizeof(LongSegmentDescriptor));

        gdt[1].limitLow = 0xFFFF;
        gdt[1].accessByte = PRESENT | NOT_SYS | EXEC | RW;
        gdt[1].flags = GRAN | LONG;
        gdt[1].limitHigh = 0xF;

        gdt[2].limitLow = 0xFFFF;
        gdt[2].accessByte = PRESENT | NOT_SYS | RW;
        gdt[2].flags = GRAN | SIZE;
        gdt[2].limitHigh = 0xF;

        // null descriptor here

        gdt[4].limitLow = 0xFFFF;
        gdt[4].accessByte = PRESENT | NOT_SYS | RW | 0b01100000; // DPL 3
        gdt[4].flags = GRAN | SIZE;
        gdt[4].limitHigh = 0xF;

        gdt[5].limitLow = 0xFFFF;
        gdt[5].accessByte = PRESENT | NOT_SYS | EXEC | RW | 0b01100000; // DPL 3
        gdt[5].flags = GRAN | LONG;
        gdt[5].limitHigh = 0xF;

        Scheduler::CPU *cpu = Scheduler::getCurrentCPU();
        assert(cpu);

        LongSegmentDescriptor *tss = (LongSegmentDescriptor *)&gdt[6];
        tss->limitLow = sizeof(TSS);
        uint64 TSSAddress = (uint64)&cpu->TSS;
        tss->baseLow = (uint16)TSSAddress;
        tss->baseMid = (uint8)(TSSAddress >> 16);
        tss->baseHigh = (uint8)(TSSAddress >> 24);
        tss->baseVeryHigh = (uint32)(TSSAddress >> 32);
        tss->accessByte = 0x89; // TSS
        tss->flags = LONG;

        cpu->gdt.base = (uint64)gdt;
        cpu->gdt.limit = 6 * sizeof(SegmentDescriptor) + sizeof(LongSegmentDescriptor) - 1;
        __asm__ volatile("lgdt (%%rax)" ::"a"(&cpu->gdt));
        __asm__ volatile("mov $(6 * 8), %ax\n"
                         "ltr %ax");

        cpu->TSS.IST1 = (uint64)kmalloc(4096);
    }

} // namespace Gdt
