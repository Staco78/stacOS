#include <devices/apic.h>
#include <memory.h>
#include <lib/list.h>
#include <lib/binaryTree.h>

namespace Devices
{
    namespace IOAPIC
    {
        static List<IOAPIC> IOApics;
        static BinaryTree<uint8, uint8> IRQMap;

        void writeIOAPIC(IOAPIC *ioapic, uint32 reg, uint32 value)
        {
            // write to the selector first.
            *((volatile uint32 *)ioapic->address) = reg & 0xFF;

            // write to the register.
            *((volatile uint32 *)(ioapic->address + 0x10)) = value;
        }

        uint32 readIOAPIC(IOAPIC *ioapic, uint32 reg)
        {
            // write to the selector first.
            *((volatile uint32 *)ioapic->address) = reg & 0xFF;

            // return the value
            return *((volatile uint32 *)(ioapic->address + 0x10));
        }

        bool isEnable()
        {
            return !IOApics.empty();
        }

        void add(uint8 ID, uint32 address, uint32 GSI)
        {
            IOAPIC ioa;
            ioa.ID = ID;
            ioa.address = Memory::Virtual::getKernelVirtualAddress(address);
            ioa.GSIBase = GSI;
            ioa.maxIRQ = readIOAPIC(&ioa, 1) >> 16 & 0xff;

            IOApics.push(ioa);
        }

        void remapIRQ(uint8 source, uint8 destination)
        {
            IRQMap.insert(source, destination);
        }

        uint8 getIRQRemap(uint8 irq)
        {
            if (!isEnable())
                return irq;
            uint8 *r = IRQMap.find(irq);
            if (r != nullptr)
                return *r;
            return irq;
        }

        IOAPIC *getIoApicForIrq(uint8 irq)
        {
            for (uint i = 0; i < IOApics.size(); i++)
            {
                IOAPIC &ioa = IOApics[i];
                if (irq >= ioa.GSIBase && irq < ioa.GSIBase + ioa.maxIRQ)
                    return &ioa;
            }

            return nullptr;
        }

        // vector start at 0 not 32
        void addIRQMapping(uint8 irq, uint8 vector, uint8 lApicID)
        {
            vector += 32;

            uint32 low = vector | 1 << 16; // masked by default
            uint32 high = (lApicID & 0xF) << 24;

            IOAPIC *ioa = getIoApicForIrq(irq);

            irq -= ioa->GSIBase;

            writeIOAPIC(ioa, 0x10 + irq * 2, low);
            writeIOAPIC(ioa, 0x10 + irq * 2 + 1, high);
        }

        void maskIRQ(uint8 irq)
        {
            IOAPIC *ioa = getIoApicForIrq(irq);
            uint reg = 0x10 + irq * 2;
            writeIOAPIC(ioa, reg, readIOAPIC(ioa, reg) | 1 << 16);
        }

        void unmaskIRQ(uint8 irq)
        {
            IOAPIC *ioa = getIoApicForIrq(irq);
            uint reg = 0x10 + irq * 2;
            writeIOAPIC(ioa, reg, readIOAPIC(ioa, reg) & ~(1 << 16));
        }

    } // namespace IOAPIC

} // namespace Devices
