#include <memory.h>
#include <terminal.h>
#include <lib/mem.h>
#include <debug.h>
#include <scheduler.h>

#define SHIFT(address) ((address) >> 12)
#define UNSHIFT(address) ((address) << 12)

namespace Memory
{
    namespace Virtual
    {

        Spinlock lock;

        inline void invalidate(uint64 addr)
        {
            asm volatile("invlpg (%0)" ::"r"(addr)
                         : "memory");
        }

        void mapPage(uint64 physicalAddress, uint64 virtualAddress, uint64 flags)
        {
            physicalAddress &= ~0xFFF;

            uint pml4EntryIndex = (virtualAddress >> 39) & 0x1FF;
            PML *pml4Entry = &Scheduler::getCurrentProcess()->pml4[pml4EntryIndex];
            if (!pml4Entry->bits.present)
            {
                uint64 address = Physical::getFreePages();
                pml4Entry->raw = address | 3;
                memset((void *)getKernelVirtualAddress(address), 0, 4096);
            }

            uint pml3EntryIndex = (virtualAddress >> 30) & 0x1FF;
            PML *pml3Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml4Entry->bits.address)))[pml3EntryIndex];
            if (!pml3Entry->bits.present)
            {
                uint64 address = Physical::getFreePages();
                pml3Entry->raw = address | 3;
                memset((void *)getKernelVirtualAddress(address), 0, 4096);
            }

            uint pml2EntryIndex = (virtualAddress >> 21) & 0x1FF;
            PML *pml2Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml3Entry->bits.address)))[pml2EntryIndex];
            if (!pml2Entry->bits.present)
            {
                uint64 address = Physical::getFreePages();
                pml2Entry->raw = address | 3;
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
            PML *pml4Entry = &Scheduler::getCurrentProcess()->pml4[pml4EntryIndex];
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

        static constexpr uint64 USER_MIN_ADDRESS = 0x400000;
        static constexpr uint64 USER_MAX_ADDRESS = 0x8000'0000'0000;
        static constexpr uint64 KERNEL_MIN_ADDRESS = 0xFFFF'8080'0000'0000;

        uint64 findFreePages(uint64 size, bool user)
        {
            uint64 pml4Index = 0;
            uint64 pml3Index = 0;
            uint64 pml2Index = 0;
            uint64 maxPML4 = 512;
            if (user)
            {
                maxPML4 = (USER_MAX_ADDRESS >> 39) & 0x1FF;
                pml4Index = (USER_MIN_ADDRESS >> 39) & 0x1FF;
                pml3Index = (USER_MIN_ADDRESS >> 30) & 0x1FF;
                pml2Index = (USER_MIN_ADDRESS >> 21) & 0x1FF;
            }
            else
            {
                pml4Index = (KERNEL_MIN_ADDRESS >> 39) & 0x1FF;
                pml3Index = (KERNEL_MIN_ADDRESS >> 30) & 0x1FF;
                pml2Index = (KERNEL_MIN_ADDRESS >> 21) & 0x1FF;
            }

            uint64 found = 0;
            uint64 returnAddress;

            PML *root = Scheduler::getCurrentProcess()->pml4;

            for (; pml4Index < maxPML4; pml4Index++)
            {
                PML *pml4Entry = &root[pml4Index];
                if (pml4Entry->bits.size)
                {
                    found = 0;
                    continue;
                }

                if (!pml4Entry->bits.present)
                {
                    uint64 address = Physical::getFreePages();
                    pml4Entry->raw = address | 3;
                    memset((void *)getKernelVirtualAddress(address), 0, 4096);
                }

                for (; pml3Index < 512; pml3Index++)
                {
                    PML *pml3Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml4Entry->bits.address)))[pml3Index];
                    if (pml3Entry->bits.size)
                    {
                        found = 0;
                        continue;
                    }

                    if (!pml3Entry->bits.present)
                    {
                        uint64 address = Physical::getFreePages();
                        pml3Entry->raw = address | 3;
                        memset((void *)getKernelVirtualAddress(address), 0, 4096);
                    }

                    for (; pml2Index < 512; pml2Index++)
                    {
                        PML *pml2Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml3Entry->bits.address)))[pml2Index];
                        if (pml2Entry->bits.size)
                        {
                            found = 0;
                            continue;
                        }

                        if (!pml2Entry->bits.present)
                        {
                            uint64 address = Physical::getFreePages();
                            pml2Entry->raw = address | 3;
                            memset((void *)getKernelVirtualAddress(address), 0, 4096);
                        }

                        for (uint64 TDIndex = 0; TDIndex < 512; TDIndex++)
                        {
                            PML *TDEntry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml2Entry->bits.address)))[TDIndex];
                            if (TDEntry->bits.present)
                                found = 0;

                            else
                            {
                                if (found == 0)
                                {
                                    returnAddress = (TDIndex | ((pml2Index | ((pml3Index | (pml4Index << 9)) << 9)) << 9)) << 12;
                                }
                                if (++found == size)
                                {
                                    return makeCanonical(returnAddress);
                                }
                            }
                        }
                    }
                }
            }

            panic("Out of virtual memory");
        }

        } // namespace Virtual

} // namespace Memory
