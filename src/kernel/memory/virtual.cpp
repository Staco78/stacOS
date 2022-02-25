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

        void init(uint64 cr3)
        {
            currentCr3 = cr3;
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
                panic("mapPage: page already mapped");

            tableEntry->raw = physicalAddress | flags | 1;
        }

        uint64 getFreePages(uint count)
        {
        }

    } // namespace Virtual

} // namespace Memory
