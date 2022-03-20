#include <devices/serial.h>
#include <asm.h>
#include <synchronization/spinlock.h>

namespace Serial
{
    static constexpr uint PORT = 0x3F8; // COM 1

    void init()
    {
        outb(PORT + 1, 0x00);
        outb(PORT + 3, 0x80);
        outb(PORT + 0, 0x01);
        outb(PORT + 1, 0x00);
        outb(PORT + 3, 0x03);
        outb(PORT + 2, 0xC7);
        outb(PORT + 4, 0x03);
    }

    void print(const char c)
    {
        while (!(inb(PORT + 5) & 0x20))
            __asm__ volatile("pause");

        outb(PORT, c);
    }

    void print(const char *str)
    {
        static Spinlock lock;
        lock.lock();
        while (*str)
            print(*str++);
        lock.unlock();
    }

    void print(const String &str)
    {
        print(str.c_str());
    }
} // namespace Serial
