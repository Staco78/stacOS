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
    fs::init();
    fs::Initrd::init();
    fs::DeviceManager::init();
    KernelSymbols::install();
    Modules::init();
    initSyscalls();

    Modules::loadModule(({fs::Node* node; assert(fs::resolvePath("/initrd/ps2.ko", node)>=0); node; }));
    Modules::loadModule(({fs::Node* node; assert(fs::resolvePath("/initrd/ext2.ko", node)>=0); node; }));

    fs::mount("/", ({fs::Node* node; assert(fs::resolvePath("/initrd/ext2.bin", node)>=0); node; }));

    // Scheduler::loadProcess("/initrd/app");
    Scheduler::loadProcess("/initrd/app");

    // uint8 buffer[128];
    // fs::Node *node;
    // fs::resolvePath("/file", node);
    // node->read(0, 128, buffer);

    // Terminal::kprintf("%s", buffer);

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

    Scheduler::init();
    Scheduler::startSMP();
    Scheduler::addThread(Scheduler::createThread(Scheduler::getCurrentProcess(), (uint64)&kernelSchedulerMain, false));

    Scheduler::start();
}