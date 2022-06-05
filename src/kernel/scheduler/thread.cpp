#include <scheduler.h>
#include <memory.h>
#include <interrupts.h>
#include <lib/mem.h>

namespace Scheduler
{

    static constexpr uint64 KERNEL_STACK_SIZE = 0x1000; // 4 ko
    static constexpr uint64 USER_STACK_SIZE = 0x10000; // 64 ko

    static_assert(KERNEL_STACK_SIZE % 4096 == 0);
    static_assert(USER_STACK_SIZE % 4096 == 0);

    Thread *createThread(Process *process, uint64 entry, bool userThread)
    {
        Thread *thread = (Thread *)kmalloc(sizeof(Thread));
        thread->process = process;
        thread->id = process->threadIdCounter++;

        uint64 kernelStackSize = userThread ? KERNEL_STACK_SIZE : USER_STACK_SIZE;

        uint64 kernelVirtualStackAddress = Memory::getFreePages(kernelStackSize / 4096, Memory::Virtual::WRITE, &process->addressSpace);
        thread->kernelStack = kernelVirtualStackAddress + kernelStackSize - sizeof(Interrupts::InterruptState);
        thread->kernelStackBase = kernelVirtualStackAddress;

        Interrupts::InterruptState *state = (Interrupts::InterruptState *)thread->kernelStack;
        memset(state, 0, sizeof(Interrupts::InterruptState));

        state->rip = entry ? entry : process->entryPoint;
        state->rflags = 0x202;

        if (userThread)
        {
            state->cs = 0x2b;
            state->ss = 0x23;
            uint64 userVirtualStackAddress = Memory::getFreePages(USER_STACK_SIZE / 4096, Memory::Virtual::WRITE | Memory::Virtual::USER, &process->addressSpace);
            state->rsp = userVirtualStackAddress + USER_STACK_SIZE;
            thread->userStackBase = userVirtualStackAddress;
        }
        else
        {
            state->cs = 0x8;
            state->ss = 0x10;
            state->rsp = thread->kernelStack;
            thread->userStackBase = 0;
        }

        thread->state = ThreadState::Stopped;

        process->threads.push(thread);

        Log::info("Create thread %i in process %i", thread->id, process->id);

        return thread;
    }

    void addThread(Thread *thread)
    {
        CPU *cpu = getCurrentCPU();
        cpu->threads.lock();
        cpu->threads.push(thread);
        cpu->threads.unlock();
    }

    void exit(int status)
    {
        CPU *cpu = Scheduler::getCurrentCPU();

        Interrupts::disable();
        cpu->currentThread->state = ThreadState::Killed;
        cpu->threadsToDestroy.lock();
        cpu->threadsToDestroy.push(cpu->currentThread);
        cpu->threadsToDestroy.unlock();

        Log::info("Scheduler: thread %i of process %i exit with status %i", cpu->currentThread->id, cpu->currentProcess->id, status);

        switchNext();
        __builtin_unreachable();
    }

    void destroyThread(Thread *thread)
    {
        assert(thread);
        CPU *cpu = Scheduler::getCurrentCPU();
        assert(thread != cpu->currentThread);

        Process *process = thread->process;

        Memory::releasePages(thread->kernelStackBase, KERNEL_STACK_SIZE / 4096, &process->addressSpace);

        if (thread->process != getKernelProcess())
        {
            assert(thread->userStackBase != 0);
            Memory::releasePages(thread->userStackBase, USER_STACK_SIZE / 4096, &process->addressSpace);
        }

        process->threads.remove(thread);

        pid id = thread->id;

        kfree(thread);

        Log::info("Scheduler: destroyed thread %i of process %i", id, process->id);

        if (process->threads.empty())
        {
            destroyProcess(process);
        }
    }
} // namespace Scheduler
