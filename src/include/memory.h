#pragma once

#include <types.h>
#include <liballoc.h>
#include <multibootInformations.h>
#include <synchronization/spinlock.h>

#define KERNEL_VMA 0xFFFFFFFF80000000
#define PHYSICAL_MEMORY_ADDR 0xFFFF'8000'0000'0000

namespace Memory
{
    namespace Physical
    {
        extern Spinlock lock;
        void init(MultibootInformations::MultibootInfo *multibootStruct);
        void setUsed(uint64 index, uint64 length = 1);
        void setFree(uint64 index, uint64 length = 1);
        uint64 findFreePages(uint64 count = 1, uint64 alignment = 4096);
        uint64 getFreePages(uint64 count = 1, uint64 alignment = 4096);
        void freePages(uint64 address, uint64 count = 1);
    } // namespace Physical

    namespace Virtual
    {
        extern Spinlock lock;

        union PML
        {
            struct
            {
                uint64 present : 1;
                uint64 writable : 1;
                uint64 user : 1;
                uint64 writethrough : 1;
                uint64 nocache : 1;
                uint64 accessed : 1;
                uint64 _available1 : 1;
                uint64 size : 1;
                uint64 global : 1;
                uint64 _available2 : 3;
                uint64 address : 28;
                uint64 reserved : 12;
                uint64 _available3 : 11;
                uint64 nx : 1;
            } bits;
            uint64 raw;
        };

        enum PageFlags
        {
            WRITE = 2,
            USER = 4,
            WRITE_THROUGH = 8,
            NO_CACHE = 16,
            GLOBAL = 256,
            NX = 0x8000000000000000
        };

        constexpr inline uint64 getKernelVirtualAddress(uint64 physicalAddress)
        {
            return physicalAddress + 0xFFFF'8000'0000'0000;
        }

        struct AddressSpace
        {
            uint64 cr3;
            inline constexpr PML *pml4() { return (PML *)getKernelVirtualAddress(cr3); }
        };

        constexpr inline uint64 makeCanonical(uint64 address)
        {
            return address & 1UL << 47 ? address | 0xFFFF000000000000 : address & 0xFFFFFFFFFFFF;
        }
        void mapPage(uint64 physicalAddress, uint64 virtualAddress, uint64 flags, AddressSpace *addressSpace = nullptr);
        void unmapPage(uint64 virtualAddress);
        uint64 findFreePages(uint64 size = 1, bool user = false);
        void *allocModuleSpace(uint64 size);
        AddressSpace createAddressSpace();
    } // namespace Virtual

    namespace Heap
    {
        void init();
    } // namespace Heap

    inline void init(void *multibootStruct)
    {
        Physical::init((MultibootInformations::MultibootInfo *)multibootStruct);
        Heap::init();
    }

    uint64 getFreePages(uint64 size = 1, uint64 flags = Virtual::WRITE, Virtual::AddressSpace *addressSpace = nullptr);

    // size in pages
    uint64 allocSpace(uint64 virtualAddress, uint size, uint64 flags = Virtual::WRITE, Virtual::AddressSpace *addressSpace = nullptr);
} // namespace Memory
