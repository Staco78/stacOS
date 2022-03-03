#include <terminal.h>
#include <types.h>
#include <multibootInformations.h>
#include <interrupts.h>
#include <memory.h>
#include <acpi.h>

extern "C" void kernelMain(void *multiboot_struct, uint64 cr3)
{
    MultibootInformations::setStruct(multiboot_struct);
    Terminal::init();
    Interrupts::IDT::init();
    Interrupts::Exceptions::init();

    Memory::init(multiboot_struct, cr3);

    ACPI::init();
    Interrupts::init();
}