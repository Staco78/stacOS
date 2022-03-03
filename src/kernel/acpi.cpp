#include <acpi.h>
#include <multibootInformations.h>
#include <memory.h>
#include <lib/mem.h>
#include <terminal.h>
#include <interrupts.h>
#include <scheduler.h>
#include <devices/apic.h>

namespace ACPI
{

    void parseMADT(MADT *madt)
    {
        uint64 localAPICAddress = madt->lAPICAddress;

        uint32 offset = 0;
        while (offset < madt->length - sizeof(MADT))
        {
            MADTEntry *entry = (MADTEntry *)((uint64)madt->entries + offset);
            offset += entry->length;
            if (entry->type == LAPICAddress)
            {
                auto address = (MADTEntryLAPICAddress *)entry;
                localAPICAddress = address->address;
            }
        }

        localAPICAddress = Memory::Virtual::getKernelVirtualAddress(localAPICAddress);

        bool foundBsp = false;
        offset = 0;
        while (offset < madt->length - sizeof(MADT))
        {
            MADTEntry *entry = (MADTEntry *)((uint64)madt->entries + offset);
            offset += entry->length;
            if (entry->type == LAPIC)
            {
                auto lapic = (MADTEntryLAPIC *)entry;
                if (lapic->flags & 1)
                {
                    Scheduler::registerCPU(!foundBsp, localAPICAddress, lapic->processorID, lapic->ID);
                    foundBsp = true;
                }
            }
            else if (entry->type == IOAPIC)
            {
                auto ioa = (MADTEntryIOAPIC *)entry;
                Devices::IOAPIC::add(ioa->ID, ioa->address, ioa->GSIB);
            }
            else if (entry->type == IOAPICIRQMap)
            {
                auto IRQ = (MADTEntryIOAPICIRQMap *)entry;
                Devices::IOAPIC::remapIRQ(IRQ->IRQSource, IRQ->GSI);
            }
        }
    }

    void init()
    {
        RSDPDescriptor *rsdp = (RSDPDescriptor *)((uint64)MultibootInformations::getEntry(MultibootInformations::ACPI_OLD_RSDP));

        if (rsdp == nullptr)
        {
            rsdp = (RSDPDescriptor *)MultibootInformations::getEntry(MultibootInformations::ACPI_NEW_RDSP);
            if (rsdp == nullptr)
                return;
        }

        RSDT *rsdt = (RSDT *)Memory::Virtual::getKernelVirtualAddress(rsdp->rsdtAddress);

        uint64 i = 0;

        for (; i < (rsdt->length - sizeof(RSDT)) / 4; i++)
        {
            if (memcmp(((RSDT *)Memory::Virtual::getKernelVirtualAddress(rsdt->pointerToOtherSDT[i]))->signature, "APIC", 4))
            {
                parseMADT((MADT *)Memory::Virtual::getKernelVirtualAddress(rsdt->pointerToOtherSDT[i]));
            }
        }
    }
} // namespace ACPI
