#pragma once
#include <types.h>
#include <lib/vector.h>
#include <lib/synchronized.h>
#include <lib/string.h>
#include <gdt.h>
#include <cpu.h>
#include <fs/fs.h>

namespace Scheduler
{

    typedef uint32 pid;
    typedef uint32 tid;

    struct Thread;

    struct FdEntry
    {
        fs::Node *node;
        uint mode;
    };

    struct Process
    {
        const char *name;
        pid id;
        tid threadIdCounter = 0;

        uint64 entryPoint;

        Memory::Virtual::AddressSpace addressSpace;

        Vector<Thread *> threads;

        uint allocateFdEntry();
        Vector<FdEntry> fds = Vector<FdEntry>(10);
    };

    enum class ThreadState
    {
        Stopped = 1,
        Running,
        Killed,
    };

    struct Thread
    {
        Process *process;
        tid id;

        uint64 kernelStack;
        uint64 kernelStackBase; // doesn't change: only for freeing
        uint64 userStackBase;   // same

        ThreadState state;
    };

    struct CPU
    {
        CPU *cpu;
        uint64 temp; // used for syscall entry
        uint lockLevel = 1;
        gdt::TSS TSS;
        uint64 lApicAddress;
        uint8 ID;
        uint8 lApicID;
        bool isBsp;

        gdt::GdtPtr gdt;
        Synchronized<Queue<Thread *>> threads;
        Synchronized<Queue<Thread *>> threadsToDestroy;
        Thread *idleThread;
        Thread *currentThread;
        Process *currentProcess;
    };

    static_assert(offsetof(CPU, temp) == 8);
    static_assert(offsetof(CPU, lockLevel) == 16);
    static_assert(offsetof(CPU, TSS) == 20);

    enum SchedulerState
    {
        Unitialised = 0,
        KernelProcessInit,
        Started
    };

    extern SchedulerState schedulerState;

    void registerCPU(bool bsp, uint64 lApicAddress, uint8 ID, uint8 lApicID);
    void initCPU(CPU *cpu);
    extern "C" CPU *getCurrentCPU();
    inline bool isCPUInitialized()
    {
        return cpu::readMSR(cpu::MSR_GS_BASE) != 0;
    }
    Process *getKernelProcess();
    Process *getCurrentProcess();
    Vector<CPU *> &getAllCPUs();
    void startSMP();

    void preinit(uint64 cr3);
    void init();
    __attribute__((noreturn)) void start();
    void switchNext();
    void switchTo(Thread *thread);
    inline void yield()
    {
        __asm__ volatile("int $0xF0");
    }

    Thread *createThread(Process *process, uint64 entry = 0, bool userThread = true);
    void addThread(Thread *thread);
    void destroyThread(Thread *thread);
    __attribute__((noreturn)) void exit(int status);

    Process *createProcess(const char *name);
    Process *loadProcess(const char *path);
    void destroyProcess(Process *process);

} // namespace Scheduler

namespace ELF
{
    void loadExecutable(Scheduler::Process *process, fs::Node *file);
}