#include <scheduler.h>
#include <cpu.h>
#include <interrupts.h>
#include <lib/mem.h>
#include <log.h>

namespace Scheduler
{

    extern "C" __attribute__((noreturn)) void doSwitch(uint64 rsp, uint64 cr3);
    extern "C" void idleTask();
    extern "C" void _schedulerTickHandler();

    SchedulerState schedulerState;
    Vector<CPU *> processors;
    Process kernelProcess;

    void registerCPU(bool bsp, uint64 lApicAddress, uint8 ID, uint8 lApicID)
    {
        CPU *cpu = new CPU();
        cpu->isBsp = bsp;
        cpu->lApicAddress = lApicAddress;
        cpu->ID = ID;
        cpu->lApicID = lApicID;

        processors.push(cpu);

        if (bsp)
            initCPU(cpu);
    }

    void initCPU(CPU *cpu)
    {
        cpu::writeMSR(cpu::MSR_GS_BASE, (uint64)cpu);
        cpu->cpu = cpu;
    }

    Vector<CPU *> &getAllCPUs()
    {
        return processors;
    }

    Process *getCurrentProcess()
    {
        if (schedulerState >= SchedulerState::KernelProcessInit)
            return &kernelProcess;
        else
            return getCurrentCPU()->currentProcess;
    }

    void preinit(uint64 cr3)
    {
        kernelProcess.id = 0;
        kernelProcess.name = "kernel";
        kernelProcess.cr3 = cr3;
        kernelProcess.pml4 = (Memory::Virtual::PML *)Memory::Virtual::getKernelVirtualAddress(cr3);

        schedulerState = Scheduler::KernelProcessInit;
    }

    void init()
    {
        assert(schedulerState >= SchedulerState::KernelProcessInit);
        schedulerState = SchedulerState::Started;

        CPU *cpu = getCurrentCPU();

        cpu->threads.realloc(1024);

        cpu->currentProcess = &kernelProcess;
        cpu->idleThread = createThread(&kernelProcess, (uint64)idleTask);
        cpu->idleThread->id = 0;
        cpu->currentThread = cpu->idleThread;
    }

    extern "C" void schedulerTickHandler()
    {
        Devices::LAPIC::sendEOI();
        CPU *cpu = getCurrentCPU();
        cpu->threads.lock();
        cpu->threads.push(cpu->currentThread);
        cpu->threads.unlock();
        switchNext();
    }

    __attribute__((noreturn)) void start()
    {

        Interrupts::IDT::setEntry(32, (uint64)_schedulerTickHandler);
        Devices::LAPIC::initTimer(32, Devices::LAPIC::PERIODIC, 10'000'000); // 10 ms

        switchNext();
    }

    void balance()
    {
        uint processorsCount = processors.size();
        CPU *sortedByThreadCount[processorsCount];
        memset(sortedByThreadCount, 0, sizeof(CPU *) * processorsCount);

        for (CPU *cpu : processors)
        {
            for (uint i = 0; i < processorsCount; i++)
            {
                if (!sortedByThreadCount[i])
                {
                    sortedByThreadCount[i] = cpu;
                    break;
                }

                if (sortedByThreadCount[i]->threads.size() > cpu->threads.size())
                {
                    memmove(&sortedByThreadCount[i + 1], &sortedByThreadCount[i], (processorsCount - i - 1) * sizeof(uint64));
                    sortedByThreadCount[i] = cpu;
                    break;
                }
            }
        };

        uint i = 0;
        for (; i < processorsCount / 2; i++)
        {
            CPU *cpu1 = sortedByThreadCount[i];
            CPU *cpu2 = sortedByThreadCount[processorsCount - i - 1];

            cpu1->threads.lock();
            cpu2->threads.lock();

            while (cpu2->threads.size() - 1 > cpu1->threads.size())
            {
                if (cpu2->threads.empty())
                    break;
                cpu1->threads.push(cpu2->threads.pop());
            }

            cpu1->threads.unlock();
            cpu2->threads.unlock();
        }

        return;
    }

    __attribute__((noreturn)) void switchNext()
    {
        CPU *cpu = getCurrentCPU();

        Thread *thread = nullptr;
        cpu->threads.lock();
        if (cpu->threads.empty())
        {
            for (CPU *_cpu : processors)
            {
                if (_cpu->ID == cpu->ID)
                    continue;

                if (_cpu->threads.tryLock())
                {

                    if (_cpu->threads.empty())
                    {
                        _cpu->threads.unlock();
                        continue;
                    }
                    thread = _cpu->threads.pop();

                    _cpu->threads.unlock();
                }
            }
            if (thread == nullptr)
                switchTo(cpu->idleThread);
        }
        else
            thread = cpu->threads.pop();

        cpu->threads.unlock();

        static uint32 balanceTick = 0;
        static Spinlock lock;
        if (++balanceTick >= 100)
        {
            if (lock.tryLock())
            {
                balanceTick = 0;
                balance();
                lock.unlock();
            }
        }

        switchTo(thread);
    }

    __attribute__((noreturn)) void switchTo(Thread *thread)
    {
        CPU *cpu = getCurrentCPU();

        cpu->TSS.RSP0 = thread->kernelStack;
        cpu->currentThread = thread;
        uint64 cr3 = thread->process == cpu->currentProcess ? 0 : thread->process->cr3;
        cpu->currentProcess = thread->process;
        doSwitch(thread->kernelStack, cr3);
    }

} // namespace Scheduler
