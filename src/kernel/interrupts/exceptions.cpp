#include <interrupts.h>
#include <debug.h>
#include <devices/pic.h>
#include <scheduler.h>

namespace Interrupts
{
    namespace Exceptions
    {
        void dump(Interrupts::InterruptState *state)
        {
#define print(r)                            \
    {                                       \
        Terminal::safe::print(#r ": ");     \
        Terminal::safe::printHex(state->r); \
        Terminal::safe::print("\n");        \
    }

            Terminal::safe::println("\nDump:");

            print(rip);
            print(cs);
            print(rflags);
            print(rsp);
            print(ss);

            print(r15);
            print(r14);
            print(r13);
            print(r12);
            print(r11);
            print(r10);
            print(r9);
            print(r8);
            print(rbp);
            print(rdi);
            print(rsi);
            print(rdx);
            print(rcx);
            print(rbx);
            print(rax);

#undef print
        }

        void _0(InterruptState *state)
        {
            dump(state);
            panic("Divide by zero");
        }
        void _1(InterruptState *state)
        {
            dump(state);
            panic("Debug");
        }
        void _2(InterruptState *state)
        {
            dump(state);
            panic("Non-maskable Interrupt");
        }
        void _3(InterruptState *state)
        {
            dump(state);
            panic("Breakpoint");
        }
        void _4(InterruptState *state)
        {
            dump(state);
            panic("Overflow");
        }

        void _5(InterruptState *state)
        {
            dump(state);
            panic("Bound Range Exceeded");
        }

        void _6(InterruptState *state)
        {
            dump(state);
            panic("Invalid Opcode");
        }
        void _7(InterruptState *state)
        {
            dump(state);
            panic("Device not Available");
        }
        void _8(InterruptState *state)
        {
            dump(state);
            panic("Double Fault");
        }
        void _9(InterruptState *state)
        {
            dump(state);
            panic("Coprocessor Segment Overrun");
        }
        void _10(InterruptState *state)
        {
            dump(state);
            panic("Invalid TSS");
        }
        void _11(InterruptState *state)
        {
            dump(state);
            panic("Segment Not Present");
        }
        void _12(InterruptState *state)
        {
            dump(state);
            panic("Stack-Segment Fault");
        }
        void _13(InterruptState *state)
        {
            if (state->err & 1)
                Terminal::safe::print("external ");
            else
                Terminal::safe::print("internal ");

            uint8 tbl = ((uint8)(state->err >> 1)) & 0b11;
            if (tbl == 0)
                Terminal::safe::print("GTD ");
            else if (tbl == 1 || tbl == 3)
                Terminal::safe::print("IDT ");
            else if (tbl == 2)
                Terminal::safe::print("LDT ");
            else
            {
                Terminal::safe::print("Invalid tbl value (");
                Terminal::safe::printInt(tbl);
                Terminal::safe::print(") ");
            }

            Terminal::safe::print("Index: ");
            Terminal::safe::printInt((uint16)(state->err >> 3));
            Terminal::safe::print("\n");
            dump(state);
            panic("General Protection Fault");
        }

        void _14(InterruptState *state)
        {
            uint64 address;
            __asm__ volatile("movq %%cr2, %0"
                             : "=r"(address));

            // Output an error message.
            Terminal::safe::print("Page fault! ( ");
            if (state->err & 0x01)
            {
                if (state->err & 0x2)
                {
                    Terminal::safe::print("read-only ");
                }
                if (state->err & 0x4)
                {
                    Terminal::safe::print("user-mode ");
                }
                if (state->err & 0x8)
                {
                    Terminal::safe::print("reserved ");
                }
                if (state->err & 0x20)
                {
                    Terminal::safe::print("instruction-fetch ");
                }
                if (state->err & 0x40)
                {
                    Terminal::safe::print("protection-key-violation ");
                }
                if (state->err & 0x80)
                {
                    Terminal::safe::print("shadow-stack-access");
                }
            }
            else
                Terminal::safe::print("not present ");

            Terminal::safe::print(") at ");
            Terminal::safe::printHex(address);
            Terminal::safe::print("\n");
            dump(state);
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
