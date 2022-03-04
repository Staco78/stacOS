#include <devices/apic.h>
#include <scheduler.h>
#include <cpuid.h>
#include <types.h>

namespace Devices
{
    namespace LAPIC
    {
        constexpr uint REG_APIC_ID = 0x20;
        constexpr uint REG_EOI = 0xB0;
        constexpr uint REG_SPURIOUS = 0xF0;

        constexpr uint REG_ISR = 0x100;

        constexpr uint REG_ICR = 0x300;
        constexpr uint REG_ICR_TARGET = 0x310;

        constexpr uint REG_LVT_TIMER = 0x320;
        constexpr uint REG_LVT_LINT0 = 0x350;
        constexpr uint REG_LVT_LINT1 = 0x360;

        constexpr uint REG_TIMER_INITIAL = 0x380;
        constexpr uint REG_TIMER_CURRENT = 0x390;
        constexpr uint REG_TIMER_DIVISOR = 0x3E0;

        constexpr uint8_t TIMER_VECTOR = 0xFE;

        void writeLAPIC(uint64 base, uint reg, uint32 value)
        {
            *((volatile uint32 *)(base + reg)) = value;
        }

        uint32 readLAPIC(uint64 base, uint reg)
        {
            return *((volatile uint32 *)(base + reg));
        }

        // target is a lApic ID
        void sendIPI(uint64 base, uint8 target, uint32 value)
        {
            writeLAPIC(base, REG_ICR_TARGET, target << 24);
            writeLAPIC(base, REG_ICR, value);
            do
            {
                asm volatile("pause"
                             :
                             :
                             : "memory");
            } while (readLAPIC(base, REG_ICR) & (1 << 12));
        }

        bool isEnable()
        {
            static uint8 enable = 2;

            if (enable != 2)
                return (bool)enable;

            uint32 eax, unused, edx;
            __get_cpuid(1, &eax, &unused, &unused, &edx);
            if (!(edx & CPUID_FEAT_EDX_APIC))
            {
                enable = 0;
                return false;
            }

            if (Scheduler::getCurrentCPU()->lApicAddress == 0)
            {
                enable = 0;
                return false;
            }
            enable = 1;
            return true;
        }

        void init()
        {
            writeLAPIC(Scheduler::getCurrentCPU()->lApicAddress, REG_SPURIOUS, 0xFF | 0x100);
        }

        void sendEOI()
        {
            writeLAPIC(Scheduler::getCurrentCPU()->lApicAddress, REG_EOI, 0);
        }
    } // namespace LAPIC

} // namespace Devices
