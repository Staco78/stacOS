#pragma once

#include <types.h>
#include <liballoc.h>
#include <multibootInformations.h>

#define KERNEL_VMA 0xFFFFFFFF80000000
#define PHYSICAL_MEMORY_ADDR 0xFFFF'8000'0000'0000

namespace Memory
{
    namespace Physical
    {
        void init(MultibootInformations::MultibootInfo *multibootStruct);
        void setUsed(uint64 index, uint64 length = 1);
        void setFree(uint64 index, uint64 length = 1);
        uint64 findFreePages(uint64 count = 1, uint64 alignment = 4096);
        uint64 getFreePages(uint64 count = 1, uint64 alignment = 4096);
        void freePages(uint64 address, uint64 count = 1);
    } // namespace Physical

    namespace Virtual
    {

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

        void init(uint64 cr3);
        constexpr inline uint64 getKernelVirtualAddress(uint64 physicalAddress)
        {
            return physicalAddress + 0xFFFF'8000'0000'0000;
        }
        void mapPage(uint64 physicalAddress, uint64 virtualAddress, uint64 flags);
        void unmapPage(uint64 virtualAddress);
    } // namespace Virtual

    namespace Heap
    {
        void init();
    } // namespace Heap

    inline void init(void *multibootStruct, uint64 cr3)
    {
        Physical::init((MultibootInformations::MultibootInfo *)multibootStruct);
        Virtual::init(cr3);
        Heap::init();
    }
} // namespace Memory
