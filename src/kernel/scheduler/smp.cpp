#include <scheduler.h>
#include <lib/list.h>
#include <devices/apic.h>
#include <devices/pit.h>
#include <interrupts.h>
#include <lib/mem.h>
#include <debug.h>

extern "C" void *apBootStart;
extern "C" void *apBootEnd;
extern "C" uint64 apCr3;
extern "C" uint64 apStack;

namespace Scheduler
{

    volatile bool started = false;
    volatile uint8 apCoreIndex;

    extern "C" void apEntry()
    {
        Gdt::install();
        Interrupts::IDT::initAp();
        Devices::LAPIC::init();
        Interrupts::enable();

        Terminal::kprintf("core %i up\n", apCoreIndex);
        started = true;

        while (1)
        {
            __asm__ volatile("hlt");
        }
    }

    void startSMP()
    {
        List<CPU> &cores = Scheduler::getAllCPUs();
        if (cores.size() <= 1)
            return;

        if (!Devices::LAPIC::isEnable())
        {
            Terminal::kprintf("Warn: no LAPIC: cannot start SMP");
            return;
        }

        Interrupts::disable();

        Memory::Virtual::mapPage((uint64)&apBootStart, (uint64)&apBootStart, Memory::Virtual::WRITE);

        apCr3 = Memory::Virtual::getCr3();

        for (uint i = 0; i < cores.size(); i++)
        {
            if (cores[i]->lApicID == getCurrentCPU()->lApicID)
                continue;

            apStack = (uint64)kmalloc(16384) + 16384;
            apCoreIndex = i;

            Devices::LAPIC::sendIPI(getCurrentCPU()->lApicAddress, cores[i]->lApicID, 0x4500); // INIT
            Devices::PIT::delay(50);
            Devices::LAPIC::sendIPI(getCurrentCPU()->lApicAddress, cores[i]->lApicID, 0x4601); // SIPI

            while (!started)
                ;

            started = false;
        }

        Memory::Virtual::unmapPage((uint64)&apBootStart);

        Interrupts::enable();
    }
} // namespace Scheduler