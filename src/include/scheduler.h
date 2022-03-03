#pragma once
#include <types.h>
#include <lib/list.h>

namespace Scheduler
{
    struct CPU
    {
        bool isBsp;
        uint64 lApicAddress;
        uint8 ID;
        uint8 lApicID;
    };

    void registerCPU(bool bsp, uint64 lApicAddress, uint8 ID, uint8 lApicID);
    CPU* getCurrentCPU();
} // namespace Scheduler
