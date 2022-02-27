#include <memory.h>
#include <multibootInformations.h>
#include <terminal.h>
#include <lib/mem.h>
#include <panic.h>
#include <debug.h>

using namespace MultibootInformations;

extern uint8 _end;

namespace Memory
{
    namespace Physical
    {
        static uint64 bitmapSize;
        static uint8 *bitmap;

        bool get(uint64 index);

        void init()
        {
            MemoryMap *memoryMap = (MemoryMap *)MultibootInformations::getEntry(MEMORY_MAP);

            uint64 memorySize = 0;
            for (uint i = 0; i < memoryMap->size / memoryMap->entrySize; i++)
            {
                if (memoryMap->entries[i].type == MemoryType::available)
                {
                    if (memoryMap->entries[i].baseAddress + memoryMap->entries[i].length > memorySize)
                        memorySize = memoryMap->entries[i].baseAddress + memoryMap->entries[i].length;
                }
            }
            bitmap = &_end;
            bitmapSize = memorySize / 4096 / 8;
            if (bitmapSize & 0xFFF)
            {
                bitmapSize |= 0xFFF;
                bitmapSize++;
            }
            memset(bitmap, 0xFF, bitmapSize);

            for (uint i = 0; i < memoryMap->size / memoryMap->entrySize; i++)
            {
                if (memoryMap->entries[i].type == MemoryType::available)
                    for (uint64 y = memoryMap->entries[i].baseAddress / 4096; y < (memoryMap->entries[i].baseAddress + memoryMap->entries[i].length) / 4096; y++)
                    {
                        if (y >= ((uint64)&_end - KERNEL_VMA + bitmapSize) / 4096)
                            setFree(y);
                    }
            }
        }

        // index and length in pages
        void setUsed(uint64 index, uint64 length)
        {
            for (uint64 i = index; i < index + length; i++)
            {
                bitmap[i / 8] |= 0b10000000 >> (i % 8);
            }
        }

        // index and length in pages
        void setFree(uint64 index, uint64 length)
        {
            for (uint64 i = index; i < index + length; i++)
            {
                bitmap[i / 8] &= ~(0b10000000 >> (i % 8));
            }
        }

        bool get(uint64 index)
        {
            return (bool)(bitmap[index / 8] & (0b10000000 >> (index % 8)));
        }

        uint64 findFreePages(uint64 count, uint64 alignment)
        {
            assert(count > 0);
            uint64 index;
            uint64 size = 0;
            for (uint64 i = 0; i < bitmapSize * 8; i++)
            {
                if (!get(i))
                {
                    if (size == 0)
                    {
                        if ((i * 4096) % alignment == 0)
                            index = i;
                        else
                            continue;
                    }
                    size++;
                    if (size == count)
                        return index * 4096;
                }
                else
                    size = 0;
            }
            panic("Physical memory allocator: cannot find free pages");
            __builtin_unreachable();
        }

        uint64 getFreePages(uint64 count, uint64 alignment)
        {
            uint64 pages = findFreePages(count, alignment);
            setUsed(pages / 4096, count);
            return pages;
        }

        void freePages(uint64 address, uint64 count)
        {
            assert((address & 0xFFF) == 0);
            setFree(address / 4096, count);
        }
    } // namespace Physical

} // namespace Memory
