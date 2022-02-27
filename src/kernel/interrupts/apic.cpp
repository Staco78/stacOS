#include <interrupts.h>
#include <types.h>
#include <cpuid_def.h>
#include <multibootInformations.h>
#include <terminal.h>
#include <lib/mem.h>
#include <memory.h>

#include <stddef.h>

#include <cpuid.h>

namespace Interrupts
{
    namespace APIC
    {

        bool isEnable()
        {

            static uint8 enable = 2;

            if (enable != 2)
                return (bool)enable;

            uint32 eax, unused, edx;
            __get_cpuid(1, &eax, &unused, &unused, &edx);
            if (!(edx & CPUID_FEAT_EDX_APIC))
            {
                enable = 0;
                return false;
            }

            RSDPDescriptor *rsdp = (RSDPDescriptor *)((uint64)MultibootInformations::getEntry(MultibootInformations::ACPI_OLD_RSDP));

            if (rsdp == nullptr)
            {
                enable = 0;
                return false;
            }

            RSDT *rsdt = (RSDT *)Memory::Virtual::getKernelVirtualAddress(rsdp->rsdtAddress);

            uint64 i = 0;

            for (; i < (rsdt->length - sizeof(RSDT)) / 4; i++)
            {
                if (memcmp(((RSDT *)Memory::Virtual::getKernelVirtualAddress(rsdt->pointerToOtherSDT[i]))->signature, "APIC", 4))
                {
                    goto found;
                }
            }

            enable = 0;
            return false;

        found:
            MADT *madt = (MADT *)Memory::Virtual::getKernelVirtualAddress(rsdt->pointerToOtherSDT[i]);

            uint64 localAPICAddress = madt->lAPICAddress;

            uint32 offset = 0;
            while (offset < madt->length - sizeof(MADT))
            {
                MADTEntry *entry = (MADTEntry *)((uint64)madt->entries + offset);
                offset += entry->length;
                if (entry->type == 5)
                {
                    localAPICAddress = *(uint64 *)((uint64)entry + 4);
                    break;
                }
            }

            localAPICAddress = Memory::Virtual::getKernelVirtualAddress(localAPICAddress);

            Terminal::kprintf("%x", localAPICAddress);

            enable = 1;
            return true;
        }
    } // namespace APIC

} // namespace Interrupts
