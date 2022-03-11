#include <interrupts.h>
#include <panic.h>
#include <devices/pic.h>

namespace Interrupts
{
    namespace Exceptions
    {
        void _0()
        {
            panic("Divide by zero");
        }
        void _1()
        {
            panic("Debug");
        }
        void _2()
        {
            panic("Non-maskable Interrupt");
        }
        void _3()
        {
            panic("Breakpoint");
        }
        void _4()
        {
            panic("Overflow");
        }

        void _5()
        {
            panic("Bound Range Exceeded");
        }

        void _6()
        {
            panic("Invalid Opcode");
        }
        void _7()
        {
            panic("Device not Available");
        }
        void _8()
        {
            panic("Double Fault");
        }
        void _9()
        {
            panic("Coprocessor Segment Overrun");
        }
        void _10()
        {
            panic("Invalid TSS");
        }
        void _11()
        {
            panic("Segment Not Present");
        }
        void _12()
        {
            panic("Stack-Segment Fault");
        }
        void _13(uint32 e)
        {
            if (e & 1)
                Terminal::kprintf("external ");
            else
                Terminal::kprintf("internal ");

            uint8 tbl = ((uint8)(e >> 1)) & 0b11;
            if (tbl == 0)
                Terminal::kprintf("GTD ");
            else if (tbl == 1 || tbl == 3)
                Terminal::kprintf("IDT ");
            else if (tbl == 2)
                Terminal::kprintf("LDT ");
            else

                Terminal::kprintf("Invalid tbl value (%i) ", tbl);

            Terminal::kprintf("Index: %i\n", (uint16)(e >> 3));
            panic("General Protection Fault");
        }
        void _14(uint32 e)
        {
            uint64 address;
            __asm__ volatile("movq %%cr2, %0"
                             : "=r"(address));

            // Output an error message.
            Terminal::kprintf("Page fault! ( ");
            if (e & 0x01)
            {
                if (e & 0x2)
                {
                    Terminal::kprintf("read-only ");
                }
                if (e & 0x4)
                {
                    Terminal::kprintf("user-mode ");
                }
                if (e & 0x8)
                {
                    Terminal::kprintf("reserved ");
                }
                if (e & 0x20)
                {
                    Terminal::kprintf("instruction-fetch ");
                }
                if (e & 0x40)
                {
                    Terminal::kprintf("protection-key-violation ");
                }
                if (e & 0x80)
                {
                    Terminal::kprintf("shadow-stack-access");
                }
            }
            else
                Terminal::kprintf("not present ");

            Terminal::kprintf(") at %x\n", address);
            panic("Page fault");
        }

        void init()
        {
            Devices::PIC::init(); // remap IRQ and disable it

            IDT::setEntry(0, (uint64)_0, 1);
            IDT::setEntry(1, (uint64)_1, 1);
            IDT::setEntry(2, (uint64)_2, 1);
            IDT::setEntry(3, (uint64)_3, 1);
            IDT::setEntry(4, (uint64)_4, 1);
            IDT::setEntry(5, (uint64)_5, 1);
            IDT::setEntry(6, (uint64)_6, 1);
            IDT::setEntry(7, (uint64)_7, 1);
            IDT::setEntry(8, (uint64)_8, 1);
            IDT::setEntry(9, (uint64)_9, 1);
            IDT::setEntry(10, (uint64)_10, 1);
            IDT::setEntry(11, (uint64)_11, 1);
            IDT::setEntry(12, (uint64)_12, 1);
            IDT::setEntry(13, (uint64)_13, 1);
            IDT::setEntry(14, (uint64)_14, 1);
        }
    } // namespace Exceptions

} // namespace Interrupts
