#include <multibootInformations.h>
#include <terminal.h>
#include <lib/mem.h>
#include <panic.h>

// case (uint32)TagType::FRAMEBUFFER_INFO:
//                 memcpy(&framebufferInfo, (void *)(addr + 8), sizeof(FramebufferInfo));
//                 if (framebufferInfo.type != 2)
//                     panic("Unsupported video mode");
//                 break;

//             case (uint32)TagType::MEMORY_MAP:
//                 memoryMap = (MemoryMap *)(addr + 4);
//                 break;

namespace MultibootInformations
{

    MultibootInfo *multibootStruct;

    void setStruct(void *multibootStruct)
    {
        MultibootInformations::multibootStruct = (MultibootInfo *)multibootStruct;
    }

    void *getEntry(uint32 type)
    {
        uint64 addr = (uint64)multibootStruct + 8;
        while (*((uint32 *)addr) != 0)
        {
            if (*(uint32 *)addr == type)
                return (void *)addr;

            addr += *(uint32 *)(addr + 4);
            if (addr & 0b111)
            {
                addr &= ~0b111;
                addr += 8;
            }
        }

        return nullptr;
    }
} // namespace MultibootInformations
