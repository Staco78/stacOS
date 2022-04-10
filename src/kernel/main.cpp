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

    KernelSymbols::install();
    Modules::init();
    Modules::loadModule((fs::FileNode *)fs::resolvePath("/initrd/testModule.ko"));

    Scheduler::init();
    Scheduler::startSMP();
    Scheduler::start();
}