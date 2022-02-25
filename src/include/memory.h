#pragma once

#include <types.h>

#define KERNEL_VMA 0xFFFFFFFF80000000

namespace Memory
{
    namespace Physical
    {
        void init();
        void setUsed(uint64 index, uint64 length = 1);
        void setFree(uint64 index, uint64 length = 1);
        uint64 findFreePages(uint64 count = 1, uint64 alignment = 4096);
        uint64 getFreePages(uint64 count = 1, uint64 alignment = 4096);
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
                uint64 cowPending : 1;
                uint64 _available2 : 2;
                uint64 address : 28;
                uint64 reserved : 12;
                uint64 _available3 : 11;
                uint64 nx : 1;
            } bits;
            uint64 raw;
        };

        void init(uint64 cr3);
        inline uint64 getKernelVirtualAddress(uint64 physicalAddress)
        {
            return physicalAddress + KERNEL_VMA;
        }
        void mapPage(uint64 physicalAddress, uint64 virtualAddress, uint64 flags);
    } // namespace Virtual

    inline void init(uint64 cr3)
    {
        Physical::init();
        Virtual::init(cr3);
    }
} // namespace Memory
