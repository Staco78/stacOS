#include <terminal.h>
#include <cpu.h>
#include <log.h>
#include <scheduler.h>
#include <errno.h>

extern "C" void syscall_entry();

void initSyscalls()
{
    cpu::writeMSR(cpu::MSR_EFER, cpu::readMSR(cpu::MSR_EFER) | 0x1);

    uint64 star = 8UL << 32 | 0x1BUL << 48;
    cpu::writeMSR(cpu::MSR_STAR, star);

    cpu::writeMSR(cpu::MSR_LSTAR, (uint64)syscall_entry);

    cpu::writeMSR(cpu::MSR_SF_MASK, 0x600); // interrupt and direction

    Log::info("Syscalls initialized");
}

extern "C"
{
    __attribute__((noreturn)) void sysExit(int status)
    {
        Scheduler::exit(status);
    }

    uint64 sysWrite(uint fd, void *buffer, uint64 size)
    {
        if (!Memory::checkAccess((uint64)buffer, size))
            return -EFAULT;
        Terminal::printStr((char *)buffer, size);
        return 0;
    }
}