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

        extern "C"
        {
            void isr_0();
            void isr_1();
            void isr_2();
            void isr_3();
            void isr_4();
            void isr_5();
            void isr_6();
            void isr_7();
            void isr_8();
            void isr_9();
            void isr_10();
            void isr_11();
            void isr_12();
            void isr_13();
            void isr_14();
            void isr_15();
            void isr_16();
            void isr_17();
            void isr_18();
            void isr_19();
            void isr_20();
            void isr_21();
            void isr_22();
            void isr_23();
            void isr_24();
            void isr_25();
            void isr_26();
            void isr_27();
            void isr_28();
            void isr_29();
            void isr_30();
            void isr_31();
            void irq_0();
            void irq_1();
            void irq_2();
            void irq_3();
            void irq_4();
            void irq_5();
            void irq_6();
            void irq_7();
            void irq_8();
            void irq_9();
            void irq_10();
            void irq_11();
            void irq_12();
            void irq_13();
            void irq_14();
            void irq_15();
        }

        void init()
        {
            idtp.base = (uint64)&idt;
            idtp.limit = (sizeof(Entry) * maxEntries) - 1;

            memset(idt, 0, sizeof(Entry) * maxEntries);

            setEntry(0, isr_0, 1);
            setEntry(1, isr_1, 1);
            setEntry(2, isr_2, 1);
            setEntry(3, isr_3, 1);
            setEntry(4, isr_4, 1);
            setEntry(5, isr_5, 1);
            setEntry(6, isr_6, 1);
            setEntry(7, isr_7, 1);
            setEntry(8, isr_8, 1);
            setEntry(9, isr_9, 1);
            setEntry(10, isr_10, 1);
            setEntry(11, isr_11, 1);
            setEntry(12, isr_12, 1);
            setEntry(13, isr_13, 1);
            setEntry(14, isr_14, 1);
            setEntry(15, isr_15, 1);
            setEntry(16, isr_16, 1);
            setEntry(17, isr_17, 1);
            setEntry(18, isr_18, 1);
            setEntry(19, isr_19, 1);
            setEntry(20, isr_20, 1);
            setEntry(21, isr_21, 1);
            setEntry(22, isr_22, 1);
            setEntry(23, isr_23, 1);
            setEntry(24, isr_24, 1);
            setEntry(25, isr_25, 1);
            setEntry(26, isr_26, 1);
            setEntry(27, isr_27, 1);
            setEntry(28, isr_28, 1);
            setEntry(29, isr_29, 1);
            setEntry(30, isr_30, 1);
            setEntry(31, isr_31, 1);

            setEntry(32, irq_0);
            setEntry(33, irq_1);
            setEntry(34, irq_2);
            setEntry(35, irq_3);
            setEntry(36, irq_4);
            setEntry(37, irq_5);
            setEntry(38, irq_6);
            setEntry(39, irq_7);
            setEntry(40, irq_8);
            setEntry(41, irq_9);
            setEntry(42, irq_10);
            setEntry(43, irq_11);
            setEntry(44, irq_12);
            setEntry(45, irq_13);
            setEntry(46, irq_14);
            setEntry(47, irq_15);

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
