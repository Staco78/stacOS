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

        uint64 kernelVirtualStackAddress = Memory::getFreePages(KERNEL_STACK_SIZE / 4096);
        thread->kernelStack = kernelVirtualStackAddress + KERNEL_STACK_SIZE - sizeof(Interrupts::InterruptState);

        Interrupts::InterruptState *state = (Interrupts::InterruptState *)thread->kernelStack;
        memset(state, 0, sizeof(Interrupts::InterruptState));

        state->rip = entry;
        state->cs = 8;
        state->ss = 0x10;
        state->rflags = 0x202;

        if (userThread)
        {
            uint64 userVirtualStackAddress = Memory::getFreePages(USER_STACK_SIZE / 4096, Memory::Virtual::WRITE | Memory::Virtual::USER);
            state->rsp = userVirtualStackAddress + USER_STACK_SIZE;
        }
        else
        {
            state->rsp = thread->kernelStack;
        }

        return thread;
    }
} // namespace Scheduler
