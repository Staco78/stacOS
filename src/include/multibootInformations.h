#pragma once
#include <types.h>

namespace MultibootInformations
{

    struct FramebufferInfo
    {
        uint32 type;
        uint32 size;
        uint64 address;
        uint32 pitch;
        uint32 width;
        uint32 height;
        uint8 bpp;
        uint8 framebuffer_type;
    } __attribute__((packed));

    struct MemoryMap
    {
        uint32 type;
        uint32 size;
        uint32 entrySize;
        uint32 entryVersion;
        struct
        {
            uint64 baseAddress;
            uint64 length;
            uint32 type;
            uint32 reserved;
        } __attribute__((packed)) entries[0];
    } __attribute__((packed));

    enum MemoryType : uint32
    {
        reserved = 0,
        available = 1,
        alsoReserved = 2,
        ACPI = 3,
        hibernation = 4,
        defective = 5
    };

    struct MultibootTag
    {
        uint32 type;
        uint32 size;
    };

    struct MultibootInfo
    {
        uint32 total_size;
        uint32 reserved;
        MultibootTag tags[0];
    };

    enum TagType : uint32
    {
        END = 0,
        COMMAND_LINE = 1,
        BOOTLOADER_NAME = 2,
        MODULES = 3,
        MEMORY_SIZE = 4,
        BOOT_DEVICE = 5,
        MEMORY_MAP = 6,
        VBE_INFO = 7,
        FRAMEBUFFER_INFO = 8,
        ELF_SYMBOLS = 9,
        APM_TABLE = 10,
        i386_EFI_SYSTEM_TABLE = 11,
        x64_EFI_SYSTEM_TABLE = 12,
        SMBIOS_TABLES = 13,
        ACPI_OLD_RSDP = 14,
        ACPI_NEW_RDSP = 15,
        NETWORK_INFOS = 16,
        EFI_MEMORY_MAP = 17,
        EFI_BOOT_SERVICE_NOT_TERMINATED = 18,
        EFI_i386_IMAGE_HANDLE = 19,
        EFI_x64_IMAGE_HANDLE = 20,
        IMAGE_LOAD_ADDRESS = 21
    };

    void setStruct(void *multibootStruct);
    void *getEntry(uint32 type);
} // namespace MultibootInformations
