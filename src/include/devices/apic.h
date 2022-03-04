#pragma once
#include <types.h>

namespace Devices
{
    namespace IOAPIC
    {
        struct IOAPIC
        {
            uint8 ID;
            uint64 address;
            uint32 GSIBase;
            uint maxIRQ;
        };
        bool isEnable();
        void add(uint8 ID, uint32 address, uint32 GSI);
        void remapIRQ(uint8 source, uint8 destination);
        uint8 getIRQRemap(uint8 irq);
        void addIRQMapping(uint8 irq, uint8 vector, uint8 lApicID);
        void maskIRQ(uint8 irq);
        void unmaskIRQ(uint8 irq);
    } // namespace IOAPIC

    namespace LAPIC
    {
        bool isEnable();
        void init();
        void sendEOI();
        void sendIPI(uint64 base, uint8 target, uint32 value);
    } // namespace LAPIC

} // namespace Devices
