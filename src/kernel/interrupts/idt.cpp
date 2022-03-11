#include <interrupts.h>
#include <lib/mem.h>
#include <terminal.h>
#include <debug.h>

namespace Interrupts
{
    namespace IDT
    {

        struct Entry
        {
            uint16 baseLow;
            uint16 segmentSelector;
            uint8 IST;
            struct
            {
                uint8 gateType : 4;
                uint8 reserved : 1;
                uint8 DPL : 2;
                uint8 present : 1;
            } __attribute__((packed)) flags;
            uint16 baseMid;
            uint32 basHigh;
            uint32 reserved;
        } __attribute__((packed));

        struct IDTPointer
        {
            uint16 limit;
            uint64 base;

        } __attribute__((packed));

        constexpr uint32 maxEntries = 256;
        static Entry idt[maxEntries];
        static IDTPointer idtp;

        void init()
        {
            idtp.base = (uint64)&idt;
            idtp.limit = (sizeof(Entry) * maxEntries) - 1;

            memset(idt, 0, sizeof(Entry) * maxEntries);
            __asm__ volatile("lidt %0" ::"m"(idtp));
        }

        void initAp()
        {
            __asm__ volatile("lidt %0" ::"m"(idtp));
        }

        void setEntry(uint8 entry, uint64 isr, uint8 ist)
        {
            assert(ist <= 7);
            idt[entry].baseLow = isr & 0xFFFF;
            idt[entry].baseMid = (isr >> 16) & 0xFFFF;
            idt[entry].basHigh = (isr >> 32) & 0xFFFFFFFF;

            idt[entry].segmentSelector = 8;

            idt[entry].flags.gateType = 0xE;
            idt[entry].flags.present = 1;
            idt[entry].IST = ist;
        }
    } // namespace IDT

} // namespace Interrupts
