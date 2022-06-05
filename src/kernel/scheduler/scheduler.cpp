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
    extern "C" void _schedulerYieldHandler();

    SchedulerState schedulerState;
    Vector<CPU *> processors;
    Process kernelProcess;

    Thread killedThread = {.process = &kernelProcess, .id = 0, .kernelStack = 0, .state = ThreadState::Killed};

    void threadDestroyerOfThreads()
    {
        CPU *cpu = getCurrentCPU();

        while (true)
        {
            while (!cpu->threadsToDestroy.empty())
            {
                Thread *thread = cpu->threadsToDestroy.pop();
                assert(thread->state == ThreadState::Killed);
                cpu->threads.lock();
                cpu->threads.replaceAll(thread, &killedThread);
                cpu->threads.unlock();

                destroyThread(thread);
            }

            yield();
        }
    }

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
        cpu->currentProcess = &kernelProcess;
    }

    Vector<CPU *> &getAllCPUs()
    {
        return processors;
    }

    Process *getKernelProcess()
    {
        assert(schedulerState >= SchedulerState::KernelProcessInit);
        return &kernelProcess;
    }

    Process *getCurrentProcess()
    {
        if (schedulerState == Scheduler::Started)
            return getCurrentCPU()->currentProcess;
        if (schedulerState >= SchedulerState::KernelProcessInit)
            return &kernelProcess;
        return nullptr;
    }

    void preinit(uint64 cr3)
    {
        kernelProcess.id = 0;
        kernelProcess.name = "kernel";
        kernelProcess.addressSpace.cr3 = cr3;

        schedulerState = Scheduler::KernelProcessInit;
    }

    void init()
    {
        assert(schedulerState >= SchedulerState::KernelProcessInit);
        schedulerState = SchedulerState::Started;

        CPU *cpu = getCurrentCPU();

        new (&cpu->threads) Synchronized<Queue<Thread *>>();
        cpu->threads.realloc(1024);

        new (&cpu->threadsToDestroy) Synchronized<Queue<Thread *>>();
        cpu->threadsToDestroy.realloc(512);

        if (cpu->isBsp)
        {
            Interrupts::IDT::setEntry(0xF0, _schedulerYieldHandler);
            new (&kernelProcess.threads) Vector<Thread *>(10);
            kernelProcess.threads.clear();
            new (&kernelProcess.fds) Vector<FdEntry>(10);
            kernelProcess.threads.clear();
        }

        cpu->idleThread = createThread(&kernelProcess, (uint64)idleTask, false);
        cpu->currentThread = cpu->idleThread;

        cpu->threads.push(createThread(&kernelProcess, (uint64)threadDestroyerOfThreads, false));
    }

    extern "C" __attribute__((noreturn)) void schedulerTickHandler(uint64 rsp)
    {
        Devices::LAPIC::sendEOI();
        CPU *cpu = getCurrentCPU();
        cpu->lockLevel = 1;
        assert(cpu->currentThread);
        assert(cpu->currentThread->state == ThreadState::Running);
        cpu->currentThread->state = ThreadState::Stopped;
        cpu->currentThread->kernelStack = rsp;
        cpu->threads.lock();
        cpu->threads.push(cpu->currentThread);
        cpu->threads.unlock();
        switchNext();
    }

    extern "C" __attribute__((noreturn)) void schedulerYieldHandler(uint64 rsp)
    {
        CPU *cpu = getCurrentCPU();
        cpu->lockLevel = 1;
        assert(cpu->currentThread);
        assert(cpu->currentThread->state == ThreadState::Running);
        cpu->currentThread->state = ThreadState::Stopped;
        cpu->currentThread->kernelStack = rsp;
        cpu->threads.lock();
        cpu->threads.push(cpu->currentThread);
        cpu->threads.unlock();
        switchNext();
    }

    __attribute__((noreturn)) void start()
    {

        Interrupts::IDT::setEntry(32, _schedulerTickHandler);
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
        __asm__("cli");
        CPU *cpu = getCurrentCPU();
        cpu->lockLevel = 1;

        assert(cpu->currentThread);
        if (cpu->currentThread->state == ThreadState::Running)
            cpu->currentThread->state = ThreadState::Stopped;

        Thread *thread = nullptr;
        cpu->threads.lock();
        if (cpu->threads.empty())
        {
            for (CPU *_cpu : processors)
            {
                if (_cpu->ID == cpu->ID)
                    continue;

                if (_cpu->threads.empty())
                    continue;

                if (_cpu->threads.tryLock())
                {
                    while (!_cpu->threads.empty() && !thread)
                    {
                        thread = _cpu->threads.pop();
                        assert(thread);
                        if (thread->state != ThreadState::Stopped)
                        {
                            if (thread->state != ThreadState::Killed)
                                _cpu->threads.push(thread);
                            thread = nullptr;
                        }
                    }
                    _cpu->threads.unlock();

                    if (thread)
                        break;
                }
            }
        }
        else
        {
            while (!cpu->threads.empty() && !thread)
            {
                thread = cpu->threads.pop();
                assert(thread);
                if (thread->state != ThreadState::Stopped)
                {
                    if (thread->state != ThreadState::Killed)
                        cpu->threads.push(thread);
                    thread = nullptr;
                }
            }
        }

        cpu->threads.unlock();

        if (thread == nullptr)
        {
            assert(cpu->idleThread);
            switchTo(cpu->idleThread);
        }

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

        assert(thread);
        switchTo(thread);
    }

    __attribute__((noreturn)) void switchTo(Thread *thread)
    {
        assert(thread);
        CPU *cpu = getCurrentCPU();

        cpu->TSS.RSP0 = thread->kernelStack + sizeof(Interrupts::InterruptState);
        cpu->currentThread = thread;
        uint64 cr3 = thread->process == cpu->currentProcess ? 0 : thread->process->addressSpace.cr3;
        cpu->currentProcess = thread->process;

        assert(cpu->currentThread->state == ThreadState::Stopped);
        cpu->currentThread->state = ThreadState::Running;
        cpu->lockLevel = 0;

        doSwitch(thread->kernelStack, cr3);
    }

} // namespace Scheduler
