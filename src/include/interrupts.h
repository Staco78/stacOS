#pragma once
#include <types.h>

namespace Interrupts
{
    namespace IDT
    {
        void init();
        void setEntry(uint8 entry, uint64 isr);
    } // namespace IDT
    
    namespace Exceptions
    {
        void init();
    } // namespace Exceptions
    

    namespace APIC
    {
        struct RSDPDescriptor
        {
            uint32 type;
            uint32 size;

            char signature[8];
            uint8 checksum;
            char OEMID[6];
            uint8 revision;
            uint32 rsdtAddress;
        } __attribute__((packed));

        struct ACPISDTHeader
        {
            char signature[4];
            uint32 length;
            uint8 revision;
            uint8 checksum;
            char OEMID[6];
            char OEMTableID[8];
            uint32 OEMRevision;
            uint32 creatorID;
            uint32 creatorRevision;
        } __attribute__((packed));

        struct RSDT : ACPISDTHeader
        {
            uint32 pointerToOtherSDT[0];
        } __attribute__((packed));

        struct RDSPDescriptor2
        {
            RSDPDescriptor firstPart;

            uint32 length;
            uint64 xsdtAddress;
            uint8 extendedChecksum;
            uint8 reserved[3];
        } __attribute__((packed));

        struct MADTEntry
        {
            uint8 type;
            uint8 length;
        } __attribute__((packed));

        struct MADTEntryLAPIC : MADTEntry
        {
            uint8 processorID;
            uint8 ID;
            uint32 flags;
        } __attribute__((packed));

        struct MADTEntryIOAPIC : MADTEntry
        {
            uint8 ID;
            uint8 reserved;
            uint32 address;
            uint32 GSIB;
        } __attribute__((packed));

        struct MADT : ACPISDTHeader
        {
            uint32 lAPICAddress;
            uint32 flags;

            MADTEntry entries[0];
        } __attribute__((packed));

        bool isEnable();
    } // namespace APIC

    void init();
    inline void enable()
    {
        __asm__("sti");
    }

    inline void disable()
    {
        __asm__("cli");
    }

} // namespace Interrupts
