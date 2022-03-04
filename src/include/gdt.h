#pragma once
#include <types.h>

namespace Gdt
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

    void install();
} // namespace Gdt
