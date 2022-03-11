#pragma once
#include <types.h>

namespace cpu
{

    constexpr uint32_t MSR_APIC_BASE = 0x1B;

    constexpr uint32_t MSR_MTRR_CAP = 0x00FE;

    constexpr uint32_t MSR_MTRR_PHYS_BASE_0 = 0x0200;
    constexpr uint32_t MSR_MTRR_PHYS_MASK_0 = 0x0201;
    constexpr uint32_t MSR_MTRR_PHYS_BASE_1 = 0x0202;
    constexpr uint32_t MSR_MTRR_PHYS_MASK_1 = 0x0203;
    constexpr uint32_t MSR_MTRR_PHYS_BASE_2 = 0x0204;
    constexpr uint32_t MSR_MTRR_PHYS_MASK_2 = 0x0205;
    constexpr uint32_t MSR_MTRR_PHYS_BASE_3 = 0x0206;
    constexpr uint32_t MSR_MTRR_PHYS_MASK_3 = 0x0207;
    constexpr uint32_t MSR_MTRR_PHYS_BASE_4 = 0x0208;
    constexpr uint32_t MSR_MTRR_PHYS_MASK_4 = 0x0209;
    constexpr uint32_t MSR_MTRR_PHYS_BASE_5 = 0x020A;
    constexpr uint32_t MSR_MTRR_PHYS_MASK_5 = 0x020B;
    constexpr uint32_t MSR_MTRR_PHYS_BASE_6 = 0x020C;
    constexpr uint32_t MSR_MTRR_PHYS_MASK_6 = 0x020D;
    constexpr uint32_t MSR_MTRR_PHYS_BASE_7 = 0x020E;
    constexpr uint32_t MSR_MTRR_PHYS_MASK_7 = 0x020F;
    constexpr uint32_t MSR_MTRR_DEFAULT = 0x02FF;

    constexpr uint32_t MSR_EFER = 0xC000'0080;

    constexpr uint32_t MSR_STAR = 0xC000'0081;
    constexpr uint32_t MSR_LSTAR = 0xC000'0082;
    constexpr uint32_t MSR_CSTAR = 0xC000'0083;
    constexpr uint32_t MSR_SF_MASK = 0xC000'0084;

    constexpr uint32_t MSR_FS_BASE = 0xC000'0100;
    constexpr uint32_t MSR_GS_BASE = 0xC000'0101;
    constexpr uint32_t MSR_KERNEL_GS_BASE = 0xC000'0102;

    void writeMSR(uint32 msr, uint64 value);
    uint64 readMSR(uint32 msr);

} // namespace cpu

struct Registers
{
    uint64 r15, r14, r13, r12;
    uint64 r11, r10, r9, r8;
    uint64 rbp, rdi, rsi, rdx, rcx, rbx, rax;
};
