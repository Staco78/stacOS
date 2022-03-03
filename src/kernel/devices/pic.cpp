#include <devices/pic.h>
#include <asm.h>
#include <debug.h>

namespace Devices
{
    namespace PIC
    {
        void init()
        {
            outb(0x20, 0x11);
            outb(0xA0, 0x11);
            outb(0x21, 0x20);
            outb(0xA1, 0x28);
            outb(0x21, 0x04);
            outb(0xA1, 0x02);
            outb(0x21, 0x01);
            outb(0xA1, 0x01);
            outb(0x21, 0x00);
            outb(0xA1, 0x00);

            disable();
        }

        void disable()
        {
            outb(0x21, 0xFF);
            outb(0xA1, 0xFF);
        }

        void maskIRQ(uint8 num)
        {
            uint16 port;
            if (num < 8)
            {
                port = 0x21;
            }
            else
            {
                port = 0xA1;
                num -= 8;
            }

            auto oldmask = inb(port);
            outb(port, oldmask | (1 << num));
        }

        void unmaskIRQ(uint8 num)
        {
            uint16 port;
            if (num < 8)
            {
                port = 0x21;
            }
            else
            {
                port = 0xA1;
                num -= 8;
                unmaskIRQ(2);
            }

            auto oldmask = inb(port);
            outb(port, oldmask & ~(1 << num));
        }

        void sendEOI(uint8 irq)
        {
            assert(irq <= 15);

            if (irq >= 8)
                outb(0xA0, 0x20);

            outb(0x20, 0x20);
        }
    } // namespace PIC

} // namespace Devices
