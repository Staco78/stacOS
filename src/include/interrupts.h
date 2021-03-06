#pragma once
#include <types.h>
#include <terminal.h>
#include <devices/apic.h>
#include <devices/pic.h>
#include <cpu.h>

namespace Interrupts
{

    struct InterruptState : Registers
    {
        uint64 err;
        uint64 rip, cs, rflags, rsp, ss;
    };

    namespace IDT
    {
        void init();
        void initAp();
        void setEntry(uint8 entry, uint64 isr, uint8 ist = 0);
        inline void setEntry(uint8 entry, void (*isr)(), uint8 ist = 0)
        {
            setEntry(entry, (uint64)isr, ist);
        }
    } // namespace IDT

    namespace Exceptions
    {
        void init();
    } // namespace Exceptions

    void init();
    void enable();
    void disable();

    inline void maskIRQ(uint8 irq)
    {
        if (Devices::IOAPIC::isEnable())
            Devices::IOAPIC::maskIRQ(irq);
        else
            Devices::PIC::maskIRQ(irq);
    }

    inline void unmaskIRQ(uint8 irq)
    {
        if (Devices::IOAPIC::isEnable())
            Devices::IOAPIC::unmaskIRQ(irq);
        else
            Devices::PIC::unmaskIRQ(irq);
    }

    inline void sendEOI(uint8 irq)
    {
        if (Devices::LAPIC::isEnable())
            Devices::LAPIC::sendEOI();
        else
            Devices::PIC::sendEOI(irq);
    }

    void addEntry(uint8 vector, uint64 entry);
    inline void addEntry(uint8 vector, void (*entry)())
    {
        addEntry(vector, (uint64)entry);
    }
    inline void addEntry(uint8 vector, void (*entry)(InterruptState *state))
    {
        addEntry(vector, (uint64)entry);
    }

    void removeEntry(uint8 vector);

} // namespace Interrupts
