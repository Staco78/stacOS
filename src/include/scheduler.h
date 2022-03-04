#pragma once
#include <types.h>
#include <lib/list.h>
#include <gdt.h>

namespace Scheduler
{
    struct CPU
    {
        bool isBsp;
        uint64 lApicAddress;
        uint8 ID;
        uint8 lApicID;

        Gdt::GdtPtr gdt;
    };

    void registerCPU(bool bsp, uint64 lApicAddress, uint8 ID, uint8 lApicID);
    CPU* getCurrentCPU();
    List<CPU>& getAllCPUs();

    void startSMP();
} // namespace Scheduler
