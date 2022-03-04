#include <memory.h>
#include <terminal.h>
#include <panic.h>
#include <lib/mem.h>
#include <debug.h>

#define SHIFT(address) (address >> 12)
#define UNSHIFT(address) (address << 12)

namespace Memory
{
    namespace Virtual
    {

        // physical address
        uint64 currentCr3;

        inline void invalidate(uint64 addr)
        {
            asm volatile("invlpg (%0)" ::"r"(addr)
                         : "memory");
        }

        void init(uint64 cr3)
        {
            currentCr3 = cr3;
        }

        uint64 getCr3()
        {
            return currentCr3;
        }

        void mapPage(uint64 physicalAddress, uint64 virtualAddress, uint64 flags)
        {
            physicalAddress &= ~0xFFF;

            uint pml4EntryIndex = (virtualAddress >> 39) & 0x1FF;
            PML *pml4Entry = &((PML *)getKernelVirtualAddress(currentCr3))[pml4EntryIndex];
            if (!pml4Entry->bits.present)
            {
                uint64 address = Physical::getFreePages();
                pml4Entry->raw = address | flags | 1;
                memset((void *)getKernelVirtualAddress(address), 0, 4096);
            }

            uint pml3EntryIndex = (virtualAddress >> 30) & 0x1FF;
            PML *pml3Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml4Entry->bits.address)))[pml3EntryIndex];
            if (!pml3Entry->bits.present)
            {
                uint64 address = Physical::getFreePages();
                pml3Entry->raw = address | flags | 1;
                memset((void *)getKernelVirtualAddress(address), 0, 4096);
            }

            uint pml2EntryIndex = (virtualAddress >> 21) & 0x1FF;
            PML *pml2Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml3Entry->bits.address)))[pml2EntryIndex];
            if (!pml2Entry->bits.present)
            {
                uint64 address = Physical::getFreePages();
                pml2Entry->raw = address | flags | 1;
                memset((void *)getKernelVirtualAddress(address), 0, 4096);
            }

            uint tableEntryIndex = (virtualAddress >> 12) & 0x1FF;
            PML *tableEntry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml2Entry->bits.address)))[tableEntryIndex];
            if (tableEntry->bits.present)
                // panic("mapPage: page already mapped");
                Terminal::kprintf("mapPage: page already mapped: %x\n", virtualAddress);

            tableEntry->raw = physicalAddress | flags | 1;
        }

        void unmapPage(uint64 virtualAddress)
        {
            const char *NOT_MAPED = "unmapPage: page not mapped";

            uint pml4EntryIndex = (virtualAddress >> 39) & 0x1FF;
            PML *pml4Entry = &((PML *)getKernelVirtualAddress(currentCr3))[pml4EntryIndex];
            if (!pml4Entry->bits.present)
                panic(NOT_MAPED);

            uint pml3EntryIndex = (virtualAddress >> 30) & 0x1FF;
            PML *pml3Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml4Entry->bits.address)))[pml3EntryIndex];
            if (!pml3Entry->bits.present)
                panic(NOT_MAPED);

            uint pml2EntryIndex = (virtualAddress >> 21) & 0x1FF;
            PML *pml2Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml3Entry->bits.address)))[pml2EntryIndex];
            if (!pml2Entry->bits.present)
                panic(NOT_MAPED);

            uint tableEntryIndex = (virtualAddress >> 12) & 0x1FF;
            PML *tableEntry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml2Entry->bits.address)))[tableEntryIndex];
            if (!tableEntry->bits.present)
                panic(NOT_MAPED);

            tableEntry->bits.present = 0;

            invalidate(virtualAddress);
        }

    } // namespace Virtual

} // namespace Memory
