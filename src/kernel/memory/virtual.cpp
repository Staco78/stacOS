#include <memory.h>
#include <terminal.h>
#include <lib/mem.h>
#include <debug.h>
#include <scheduler.h>
#include <lib/lockGuard.h>

#define SHIFT(address) ((address) >> 12)
#define UNSHIFT(address) ((address) << 12)

namespace Memory
{
    namespace Virtual
    {

        inline void invalidate(uint64 addr)
        {
            asm volatile("invlpg (%0)" ::"r"(addr)
                         : "memory");
        }

        void mapPage(uint64 physicalAddress, uint64 virtualAddress, uint64 flags, AddressSpace *addressSpace, bool lock)
        {
            physicalAddress &= ~0xFFF;

            if (!addressSpace)
                addressSpace = &Scheduler::getCurrentProcess()->addressSpace;

            if (lock)
                addressSpace->lock.lock();

            uint pml4EntryIndex = (virtualAddress >> 39) & 0x1FF;
            PML *pml4Entry = &addressSpace->pml4()[pml4EntryIndex];
            if (!pml4Entry->bits.present)
            {
                uint64 address = Physical::getFreePages();
                pml4Entry->raw = address | (pml4EntryIndex < 128 ? 7 : 3);
                memset((void *)getKernelVirtualAddress(address), 0, 4096);
            }

            uint pml3EntryIndex = (virtualAddress >> 30) & 0x1FF;
            PML *pml3Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml4Entry->bits.address)))[pml3EntryIndex];
            if (!pml3Entry->bits.present)
            {
                uint64 address = Physical::getFreePages();
                pml3Entry->raw = address | (pml4EntryIndex < 128 ? 7 : 3);
                memset((void *)getKernelVirtualAddress(address), 0, 4096);
            }

            uint pml2EntryIndex = (virtualAddress >> 21) & 0x1FF;
            PML *pml2Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml3Entry->bits.address)))[pml2EntryIndex];
            if (!pml2Entry->bits.present)
            {
                uint64 address = Physical::getFreePages();
                pml2Entry->raw = address | (pml4EntryIndex < 128 ? 7 : 3);
                memset((void *)getKernelVirtualAddress(address), 0, 4096);
            }

            uint tableEntryIndex = (virtualAddress >> 12) & 0x1FF;
            PML *tableEntry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml2Entry->bits.address)))[tableEntryIndex];
            if (tableEntry->bits.present)
            {
                // panic("mapPage: page already mapped");
                Log::safe::print("mapPage: page already mapped: ");
                Log::safe::printHex(virtualAddress);
                Log::safe::print("\n");
            }

            tableEntry->raw = physicalAddress | flags | 1;

            if (lock)
                addressSpace->lock.unlock();
        }

        // return physical address
        uint64 unmapPage(uint64 virtualAddress, AddressSpace *addressSpace)
        {
            if (!addressSpace)
                addressSpace = &Scheduler::getCurrentProcess()->addressSpace;

            addressSpace->lock.lock();

            const char *NOT_MAPED = "unmapPage: page not mapped";

            uint pml4EntryIndex = (virtualAddress >> 39) & 0x1FF;
            PML *pml4Entry = &addressSpace->pml4()[pml4EntryIndex];
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

            addressSpace->lock.unlock();

            invalidate(virtualAddress);

            return UNSHIFT(tableEntry->bits.address);
        }

        static constexpr uint64 USER_MIN_ADDRESS = 0x400000;
        static constexpr uint64 USER_MAX_ADDRESS = 0x8000'0000'0000;
        static constexpr uint64 KERNEL_MIN_ADDRESS = 0xFFFF'8080'0000'0000;

        uint64 findFreePages(uint64 size, bool user, AddressSpace *addressSpace, bool lock)
        {

            if (!addressSpace)
                addressSpace = &Scheduler::getCurrentProcess()->addressSpace;

            if (lock)
                addressSpace->lock.lock();

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

            PML *root = addressSpace->pml4();

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
                                    if (lock)
                                        addressSpace->lock.unlock();
                                    return makeCanonical(returnAddress);
                                }
                            }
                        }
                    }
                }
            }

            panic("Out of virtual memory");
        }

        static constexpr uint64 moduleSpaceBaseAddress = 0xFFFF'FFFE'8000'0000;
        static constexpr uint64 moduleSpaceSize = 0x8000'0000;

        uint64 moduleBaseAddressCurrent = moduleSpaceBaseAddress;

        void *allocModuleSpace(uint64 size)
        {
            if ((size & 0xFFF) != 0)
            {
                size |= 0xFFF;
                size++;
            }

            if ((moduleBaseAddressCurrent + size) > (moduleSpaceBaseAddress + moduleSpaceSize))
                return nullptr;

            void *r = (void *)moduleBaseAddressCurrent;
            for (uint i = 0; i < size / 4096; i++)
            {
                uint64 address = Physical::getFreePages();
                Virtual::mapPage(address, moduleBaseAddressCurrent, WRITE, &Scheduler::getKernelProcess()->addressSpace);
                moduleBaseAddressCurrent += 4096;
            }

            return r;
        }

        AddressSpace createAddressSpace()
        {
            AddressSpace space;
            space.cr3 = Physical::getFreePages();

            PML *kernelPML = Scheduler::getKernelProcess()->addressSpace.pml4();
            memcpy(space.pml4(), kernelPML, 4096);
            memset(space.pml4(), 0, 2048);
            return space;
        }

        // this will release all memory
        void destroyAddressSpace(AddressSpace *space)
        {
            for (uint pml4Index = 0; pml4Index < 256; pml4Index++)
            {
                PML *pml4Entry = &space->pml4()[pml4Index];
                if (!pml4Entry->bits.present)
                    continue;

                for (uint pml3Index = 0; pml3Index < 512; pml3Index++)
                {
                    PML *pml3Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml4Entry->bits.address)))[pml3Index];
                    if (!pml3Entry->bits.present)
                        continue;

                    for (uint pml2Index = 0; pml2Index < 512; pml2Index++)
                    {
                        PML *pml2Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml3Entry->bits.address)))[pml2Index];
                        if (!pml2Entry->bits.present)
                            continue;

                        for (uint TDIndex = 0; TDIndex < 512; TDIndex++)
                        {
                            PML *TDEntry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml2Entry->bits.address)))[TDIndex];
                            if (!TDEntry->bits.present)
                                continue;
                            Physical::freePages(UNSHIFT(TDEntry->bits.address));
                        }
                        Physical::freePages(UNSHIFT(pml2Entry->bits.address));
                    }
                    Physical::freePages(UNSHIFT(pml3Entry->bits.address));
                }
                Physical::freePages(UNSHIFT(pml4Entry->bits.address));
            }

            Physical::freePages(space->cr3);
        }

        PML *getPage(uint64 virtualAddress, AddressSpace *addressSpace)
        {
            if (!addressSpace)
                addressSpace = &Scheduler::getCurrentProcess()->addressSpace;

            LockGuard<Spinlock> lock(addressSpace->lock);

            uint pml4Index = (virtualAddress >> 39) & 0x1FF;
            uint pml3Index = (virtualAddress >> 30) & 0x1FF;
            uint pml2Index = (virtualAddress >> 21) & 0x1FF;
            uint TDIndex = (virtualAddress >> 12) & 0x1FF;

            PML *pml4Entry = &addressSpace->pml4()[pml4Index];
            if (!pml4Entry->bits.present)
                return nullptr;
            if (pml4Entry->bits.size)
                return nullptr;

            PML *pml3Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml4Entry->bits.address)))[pml3Index];
            if (!pml3Entry->bits.present)
                return nullptr;
            if (pml3Entry->bits.size)
                return nullptr;

            PML *pml2Entry = &((PML *)getKernelVirtualAddress(UNSHIFT(pml3Entry->bits.address)))[pml2Index];
            if (!pml2Entry->bits.present)
                return nullptr;
            if (pml2Entry->bits.size)
                return nullptr;

            return &((PML *)getKernelVirtualAddress(UNSHIFT(pml2Entry->bits.address)))[TDIndex];
        }

    } // namespace Virtual

    uint64 getFreePages(uint64 size, uint64 flags, Virtual::AddressSpace *addressSpace)
    {
        if (!addressSpace)
            addressSpace = &Scheduler::getCurrentProcess()->addressSpace;

        addressSpace->lock.lock();
        uint64 virtualAddress = Virtual::findFreePages(size, flags & Virtual::USER, addressSpace, false);
        for (uint64 i = 0; i < size * 4096; i += 4096)
        {
            Virtual::mapPage(Physical::getFreePages(), virtualAddress + i, flags, addressSpace, false);
        }
        addressSpace->lock.unlock();

        return virtualAddress;
    }

    uint64 allocSpace(uint64 virtualAddress, uint size, uint64 flags, Virtual::AddressSpace *addressSpace)
    {

        uint64 physical = Physical::getFreePages(size);
        uint64 offset = 0;
        addressSpace->lock.lock();
        for (uint i = 0; i < size; i++)
        {
            Virtual::mapPage(physical + offset, virtualAddress + offset, flags, addressSpace, false);
            offset += 4096;
        }
        addressSpace->lock.unlock();
        return physical;
    }

    void releasePages(uint64 address, uint size, Virtual::AddressSpace *addressSpace)
    {
        if (!addressSpace)
            addressSpace = &Scheduler::getCurrentProcess()->addressSpace;

        for (uint i = 0; i < size; i++)
        {
            uint64 physicalAddress = Virtual::unmapPage(address + i * 4096, addressSpace);
            Physical::freePages(physicalAddress);
        }
    }

    bool checkAccess(uint64 address, uint64 size, bool write, Virtual::AddressSpace *addressSpace)
    {
        if (!addressSpace)
            addressSpace = &Scheduler::getCurrentProcess()->addressSpace;

        if (!address)
            return false;

        if (address >= Virtual::USER_MAX_ADDRESS || (address + size) >= Virtual::USER_MAX_ADDRESS)
            return false;

        uint64 end = size ? (address + size - 1) : address;

        uint pageBase = SHIFT(address);
        uint pageEnd = SHIFT(end);

        for (uint i = pageBase; i <= pageEnd; i++)
        {
            Virtual::PML *page = Virtual::getPage(UNSHIFT(i), addressSpace);
            if (!page)
                return false;
            if (!page->bits.present)
                return false;
            if (write && !page->bits.writable)
                return false;
        }

        return true;
    }

    bool checkStrAccess(const char *str, Virtual::AddressSpace *addressSpace)
    {

        if (!addressSpace)
            addressSpace = &Scheduler::getCurrentProcess()->addressSpace;

        if (!str)
            return false;

        const char *current = str;

        while (true)
        {
            if (checkAccess((uint64)current, 10, false, addressSpace))
            {
                for (uint i = 0; i < 10; i++)
                {
                    if (!current[i])
                        return true;
                }

                current += 10;
            }
            else
            {
                for (uint i = 0; i < 10; i++)
                {
                    if (!checkAccess((uint64)(current + i), 1, false, addressSpace))
                        return false;
                    if (current[i] == 0)
                        return true;
                }
                assert(false);
            }
        }
    }

} // namespace Memory
