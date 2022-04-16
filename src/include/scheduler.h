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

    struct Process
    {
        const char *name;
        pid id;

        uint64 entryPoint;

        Memory::Virtual::AddressSpace addressSpace;

        Vector<Thread *> threads;
    };

    struct Thread
    {
        Process *process;
        tid id;

        uint64 kernelStack;
    };

    struct CPU
    {
        CPU *cpu;
        uint64 lApicAddress;
        uint8 ID;
        uint8 lApicID;
        bool isBsp;

        uint lockLevel = 1;

        gdt::GdtPtr gdt;
        gdt::TSS TSS;
        Synchronized<Queue<Thread *>> threads;
        Thread *idleThread;
        Thread *currentThread;
        Process *currentProcess;
    };

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

    Thread *createThread(Process *process, uint64 entry = 0, bool userThread = true);
    void addThread(Thread *thread);

    Process *createProcess(const char *name);
    Process *loadProcess(const char *path);

} // namespace Scheduler

namespace ELF
{
    void loadExecutable(Scheduler::Process *process, fs::FileNode *file);
}