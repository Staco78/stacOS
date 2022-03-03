#pragma once
#include <types.h>

namespace ACPI
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

    struct RSDPDescriptor2
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

    struct MADTEntryLAPIC : MADTEntry // type = 0
    {
        uint8 processorID;
        uint8 ID;
        uint32 flags;
    } __attribute__((packed));

    struct MADTEntryIOAPIC : MADTEntry // type = 1
    {
        uint8 ID;
        uint8 reserved;
        uint32 address;
        uint32 GSIB;
    } __attribute__((packed));

    struct MADTEntryIOAPICIRQMap : MADTEntry // type = 2
    {
        uint8 busSource;
        uint8 IRQSource;
        uint32 GSI;
        uint16 flags;
    } __attribute__((packed));

    struct MADTEntryIOAPICNMI : MADTEntry // type = 3
    {
        uint8 NMI;
        uint8 reserved;
        uint16 flags;
        uint32 GSI;
    } __attribute__((packed));

    struct MADTEntryLAPICNMI : MADTEntry // type = 4
    {
        uint8 processorID;
        uint16 flags;
        uint8 LINT;
    } __attribute__((packed));

    struct MADTEntryLAPICAddress : MADTEntry // type = 5
    {
        uint16 reserved;
        uint64 address;
    } __attribute__((packed));

    struct MADT : ACPISDTHeader
    {
        uint32 lAPICAddress;
        uint32 flags;

        MADTEntry entries[0];
    } __attribute__((packed));

    enum MADTEntryType : uint8
    {
        LAPIC = 0,
        IOAPIC = 1,
        IOAPICIRQMap = 2,
        IOAPICNMI = 3,
        LAPICNMI = 4,
        LAPICAddress = 5
    };

    void init();
} // namespace ACPI
