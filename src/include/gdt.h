#pragma once
#include <types.h>
#include <stddef.h>

namespace gdt
{
    struct GdtPtr
    {
        uint16 limit;
        uint64 base;
    } __attribute__((packed));

    struct SegmentDescriptor
    {
        uint16 limitLow;
        uint16 baseLow;
        uint8 baseMid;
        uint8 accessByte;
        struct
        {
            uint8 limitHigh : 4;
            uint8 flags : 4;
        } __attribute__((packed));
        uint8 baseHigh;
    } __attribute__((packed));

    struct LongSegmentDescriptor : SegmentDescriptor
    {
        uint32 baseVeryHigh;
        uint32 reserved;
    } __attribute__((packed));

    struct TSS
    {
        uint32 reserved;
        uint64 RSP0;
        uint64 RSP1;
        uint64 RSP2;
        uint64 reserved1;
        uint64 IST1;
        uint64 IST2;
        uint64 IST3;
        uint64 IST4;
        uint64 IST5;
        uint64 IST6;
        uint64 IST7;
        uint64 reserved2;
        uint16 reserved3;
        uint16 IOPB;
    } __attribute__((packed));

    static_assert(offsetof(TSS, RSP0) == 4);

    void install();

} // namespace Gdt
