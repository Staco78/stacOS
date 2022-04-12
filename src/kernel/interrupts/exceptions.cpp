#include <interrupts.h>
#include <debug.h>
#include <devices/pic.h>
#include <scheduler.h>

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
        void _13(InterruptState *state)
        {
            if (state->err & 1)
                Terminal::kprintf("external ");
            else
                Terminal::kprintf("internal ");

            uint8 tbl = ((uint8)(state->err >> 1)) & 0b11;
            if (tbl == 0)
                Terminal::kprintf("GTD ");
            else if (tbl == 1 || tbl == 3)
                Terminal::kprintf("IDT ");
            else if (tbl == 2)
                Terminal::kprintf("LDT ");
            else

                Terminal::kprintf("Invalid tbl value (%i) ", tbl);

            Terminal::kprintf("Index: %i\n", (uint16)(state->err >> 3));
            panic("General Protection Fault");
        }
        void _14(InterruptState* state)
        {
            uint64 address;
            __asm__ volatile("movq %%cr2, %0"
                             : "=r"(address));

            // Output an error message.
            Terminal::kprintf("Page fault! ( ");
            if (state->err & 0x01)
            {
                if (state->err & 0x2)
                {
                    Terminal::kprintf("read-only ");
                }
                if (state->err & 0x4)
                {
                    Terminal::kprintf("user-mode ");
                }
                if (state->err & 0x8)
                {
                    Terminal::kprintf("reserved ");
                }
                if (state->err & 0x20)
                {
                    Terminal::kprintf("instruction-fetch ");
                }
                if (state->err & 0x40)
                {
                    Terminal::kprintf("protection-key-violation ");
                }
                if (state->err & 0x80)
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

            addEntry(0, _0);
            addEntry(1, _1);
            addEntry(2, _2);
            addEntry(3, _3);
            addEntry(4, _4);
            addEntry(5, _5);
            addEntry(6, _6);
            addEntry(7, _7);
            addEntry(8, _8);
            addEntry(9, _9);
            addEntry(10, _10);
            addEntry(11, _11);
            addEntry(12, _12);
            addEntry(13, _13);
            addEntry(14, _14);
        } 
    } // namespace Exceptions

} // namespace Interrupts
