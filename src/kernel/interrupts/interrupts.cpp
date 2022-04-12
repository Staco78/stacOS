#include <interrupts.h>
#include <devices/apic.h>
#include <devices/pic.h>
#include <scheduler.h>
#include <log.h>

namespace Interrupts
{

    void init()
    {
        if (!Devices::IOAPIC::isEnable())
            Log::warn("No IOAPIC: using PIC");

        if (Devices::LAPIC::isEnable())
            Devices::LAPIC::init();
    }

    void (*entries[256])(InterruptState *state);

    extern "C" void interruptsHandler(uint vector, InterruptState *state)
    {
        assert(entries[vector]);
        entries[vector](state);
    }

    // vector between 0-255
    void addEntry(uint8 vector, uint64 entry)
    {
        if (entries[vector])
            Log::warn("set interrupt entry %i: already exist", vector);

        entries[vector] = (void (*)(InterruptState *))entry;

        if (vector >= 32)
        {
            vector -= 32;
            if (Devices::IOAPIC::isEnable())
            {
                if (Devices::IOAPIC::getIoApicForIrq(vector))
                {
                    Devices::IOAPIC::addIRQMapping(vector, vector, Scheduler::getCurrentCPU()->lApicID);
                    Devices::IOAPIC::unmaskIRQ(vector);
                }
            }
            else if (vector < 16)
            {
                Devices::PIC::unmaskIRQ(vector);
            }
        }
    }

    void removeEntry(uint8 vector)
    {
        entries[vector] = nullptr;

        if (vector >= 32)
        {
            vector -= 32;
            if (Devices::IOAPIC::isEnable())
            {
                if (Devices::IOAPIC::getIoApicForIrq(vector))
                {
                    Devices::IOAPIC::maskIRQ(vector);
                }
            }
            else if (vector < 16)
            {
                Devices::PIC::maskIRQ(vector);
            }
        }
    }
} // namespace Interrupts
