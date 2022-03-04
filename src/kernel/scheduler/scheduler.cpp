#include <scheduler.h>

namespace Scheduler
{
    List<CPU> processors;

    void registerCPU(bool bsp, uint64 lApicAddress, uint8 ID, uint8 lApicID)
    {
        CPU cpu;
        cpu.isBsp = bsp;
        cpu.lApicAddress = lApicAddress;
        cpu.ID = ID;
        cpu.lApicID = lApicID;
        processors.push(cpu);
    }

    CPU *getCurrentCPU()
    {
        return processors[0];
    }

    List<CPU> &getAllCPUs()
    {
        return processors;
    }
} // namespace Scheduler
