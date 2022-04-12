#include <memory.h>
#include <interrupts.h>
#include <stddef.h>
#include <lib/mem.h>
#include <terminal.h>
#include <synchronization/spinlock.h>
#include <log.h>

namespace Memory
{
    namespace Heap
    {

        static constexpr uint64 heapAddress = 0xFFFF'FFFF'0000'0000;
        static constexpr uint64 heapSize = 0x80000000; // 2GB

        uint64 *physicalPages; // array of address of physical pages mapped
        static constexpr uint64 physicalPagesSize = 1024 * 512;

        Spinlock lock;

        void init()
        {
            physicalPages = (uint64 *)Virtual::getKernelVirtualAddress(Physical::getFreePages(physicalPagesSize / 512));
            memset(physicalPages, 0, physicalPagesSize * 8);

            new (&lock) Spinlock();
        }
    } // namespace Heap

} // namespace Memory

using namespace Memory;
using namespace Memory::Heap;

extern "C" int liballoc_lock()
{
    Interrupts::disable();
    lock.lock();
    return 0;
}

extern "C" int liballoc_unlock()
{
    lock.unlock();
    Interrupts::enable();
    return 0;
}

extern "C" void *liballoc_alloc(size_t size)
{
    size_t count = 0;
    for (size_t i = 0; i < physicalPagesSize; i++)
    {
        if (physicalPages[i] == 0)
        {
            if (++count == size)
            {
                size_t y = 0;
                for (; y < size; y++)
                {
                    physicalPages[i - y] = Physical::getFreePages();
                    Virtual::mapPage(physicalPages[i - y], heapAddress + (i - y) * 4096, 2);
                }
                return (void *)(heapAddress + (i - --y) * 4096);
            }
        }
        else
            count = 0;
    }
    return NULL;
}

extern "C" int liballoc_free(void *ptr, size_t size)
{
    size_t baseIndex = ((uint64)ptr - heapAddress) / 4096;
    for (size_t i = baseIndex; i < size + baseIndex; i++)
    {
        Physical::freePages(physicalPages[i]);
        Virtual::unmapPage(heapAddress + i * 4096);
        physicalPages[i] = 0;
    }
    return 0;
}