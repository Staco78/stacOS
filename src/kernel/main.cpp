#include <terminal.h>
#include <types.h>
#include <multibootInformations.h>
#include <interrupts.h>
#include <memory.h>
#include <acpi.h>
#include <scheduler.h>
#include <gdt.h>
#include <devices/serial.h>
#include <fs/fs.h>
#include <symbols.h>
#include <modules.h>
#include <syscalls.h>

void kernelSchedulerMain()
{
    initSyscalls();
    KernelSymbols::install();
    Modules::init();

    Modules::loadModule((fs::FileNode *)fs::resolvePath("/initrd/ps2.ko"));

    Scheduler::loadProcess("/initrd/app");
    Scheduler::loadProcess("/initrd/app");

    Scheduler::exit(0);
}

extern "C" void kernelMain(void *multiboot_struct, uint64 cr3)
{
    Scheduler::preinit(cr3);
    Serial::init();
    MultibootInformations::setStruct(multiboot_struct);
    Terminal::init();
    Interrupts::IDT::init();
    Interrupts::Exceptions::init();

    Memory::init(multiboot_struct);

    ACPI::init();
    gdt::install();
    Interrupts::init();

    Devices::LAPIC::calibrateTimer();

    fs::Initrd::init();

    Scheduler::init();
    Scheduler::startSMP();
    Scheduler::addThread(Scheduler::createThread(Scheduler::getCurrentProcess(), (uint64)&kernelSchedulerMain, false));

    Scheduler::start();
}