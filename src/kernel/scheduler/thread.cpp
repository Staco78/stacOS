#include <scheduler.h>
#include <memory.h>
#include <interrupts.h>
#include <lib/mem.h>

namespace Scheduler
{

    static constexpr uint64 KERNEL_STACK_SIZE = 0x1000;
    static constexpr uint64 USER_STACK_SIZE = 0x2000;

    static_assert(KERNEL_STACK_SIZE % 4096 == 0);
    static_assert(USER_STACK_SIZE % 4096 == 0);

    uint32 threadIdCounter = 0;

    Thread *createThread(Process *process, uint64 entry, bool userThread)
    {
        Thread *thread = (Thread *)kmalloc(sizeof(Thread));
        thread->process = process;
        thread->id = threadIdCounter++;

        uint64 kernelVirtualStackAddress = Memory::getFreePages(KERNEL_STACK_SIZE / 4096, Memory::Virtual::WRITE, &process->addressSpace);
        thread->kernelStack = kernelVirtualStackAddress + KERNEL_STACK_SIZE - sizeof(Interrupts::InterruptState);

        Interrupts::InterruptState *state = (Interrupts::InterruptState *)thread->kernelStack;
        memset(state, 0, sizeof(Interrupts::InterruptState));

        state->rip = entry ? entry : process->entryPoint;
        state->rflags = 0x202;

        if (userThread)
        {
            state->cs = 0x1b;
            state->ss = 0x23;
            uint64 userVirtualStackAddress = Memory::getFreePages(USER_STACK_SIZE / 4096, Memory::Virtual::WRITE | Memory::Virtual::USER, &process->addressSpace);
            state->rsp = userVirtualStackAddress + USER_STACK_SIZE;
        }
        else
        {
            state->cs = 0x8;
            state->ss = 0x10;
            state->rsp = thread->kernelStack;
        }

        return thread;
    }

    void addThread(Thread *thread)
    {
        thread->process->threads.push(thread);
        CPU *cpu = getCurrentCPU();
        cpu->threads.lock();
        cpu->threads.push(thread);
        cpu->threads.unlock();
    }
} // namespace Scheduler
