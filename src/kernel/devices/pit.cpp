#include <devices/pit.h>
#include <asm.h>
#include <terminal.h>

namespace Devices
{
    namespace PIT
    {
        void delay(uint ms)
        {
            outb(0x43, 0x30); // mode 0
            uint16 value = ms * 1193;
            outb(0x40, value);
            outb(0x40, value >> 8);

            while (true)
            {
                outb(0x43, 0xE2);
                if (inb(0x40) & 128)
                    return;
                __asm__ volatile("pause");
            }
        }
    } // namespace PIT

} // namespace Devices
