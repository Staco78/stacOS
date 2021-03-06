#include <memory.h>
#include <terminal.h>
#include <lib/mem.h>
#include <debug.h>
#include <lib/lockGuard.h>

using namespace MultibootInformations;

extern uint8 _end;

namespace Memory
{
    namespace Physical
    {
        static uint64 bitmapSize;
        static uint8 *bitmap;

        extern "C" uint64 physical_pml2; // from boot.asm

        Spinlock lock;

        bool get(uint64 index);

        void init(MultibootInformations::MultibootInfo *multibootStruct)
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

            memcpy((void *)((uint64)&_end + bitmapSize), multibootStruct, multibootStruct->total_size);
            multibootStruct = (MultibootInfo *)((uint64)&_end + bitmapSize);
            MultibootInformations::setStruct((void *)multibootStruct);

            memset(bitmap, 0xFF, bitmapSize);

            uint64 multibootSize = multibootStruct->total_size;
            if (multibootSize & 0xFFF)
            {
                multibootSize |= 0xFFF;
                multibootSize++;
            }

            for (uint i = 0; i < memoryMap->size / memoryMap->entrySize; i++)
            {
                if (memoryMap->entries[i].type == MemoryType::available)
                    for (uint64 y = memoryMap->entries[i].baseAddress / 4096; y < (memoryMap->entries[i].baseAddress + memoryMap->entries[i].length) / 4096; y++)
                    {
                        if (y >= ((uint64)&_end - KERNEL_VMA + bitmapSize + multibootSize) / 4096)
                            setFree(y);
                    }
            }

            if (*(uint64 *)(Virtual::getKernelVirtualAddress((uint64)&physical_pml2)) == 0)
            {
                for (uint i = 0; i < 512; i++)
                    setFree(((uint64)&physical_pml2) / 4096 + i);
            }

            Module *module = (Module *)findModule("initrd");
            setUsed(module->start / 4096, (module->end - module->start) / 4096);
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

        uint64 findFreePages(uint64 count, uint64 alignment, bool _lock)
        {
            if (_lock)
                lock.lock();
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
                    {
                        if (_lock)
                            lock.unlock();
                        return index * 4096;
                    }
                }
                else
                    size = 0;
            }
            panic("Physical memory allocator: cannot find free pages");
            __builtin_unreachable();
        }

        uint64 getFreePages(uint64 count, uint64 alignment)
        {
            lock.lock();
            uint64 pages = findFreePages(count, alignment, false);
            setUsed(pages / 4096, count);
            lock.unlock();
            return pages;
        }

        void freePages(uint64 address, uint64 count)
        {
            assert((address & 0xFFF) == 0);
            setFree(address / 4096, count);
        }
    } // namespace Physical

} // namespace Memory
