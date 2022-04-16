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

    void enable()
    {
        if (!Scheduler::isCPUInitialized())
            __asm__ volatile("sti");
        else
        {
            Scheduler::CPU *cpu = Scheduler::getCurrentCPU();
            assert(cpu->lockLevel > 0);
            if (--cpu->lockLevel == 0)
                __asm__ volatile("sti");
        }
    }

    void disable()
    {
        __asm__ volatile("cli");
        if (Scheduler::isCPUInitialized())
            Scheduler::getCurrentCPU()->lockLevel++;
    }

    void (*entries[256])(InterruptState *state);

    extern "C" void interruptsHandler(uint vector, InterruptState *state)
    {
        Scheduler::getCurrentCPU()->lockLevel = 1;
        assert(state);
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
                    Scheduler::CPU *cpu = Scheduler::getCurrentCPU();
                    assert(cpu);
                    Devices::IOAPIC::addIRQMapping(vector, vector, cpu->lApicID);
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
