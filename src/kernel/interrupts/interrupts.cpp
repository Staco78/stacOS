#include <interrupts.h>
#include <devices/apic.h>
#include <devices/pic.h>

namespace Interrupts
{

    void init()
    {
        if (!Devices::IOAPIC::isEnable())
            Terminal::kprintf("No IOAPIC: using PIC\n");

        if (Devices::LAPIC::isEnable())
            Devices::LAPIC::init();
    }
} // namespace Interrupts
