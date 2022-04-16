#include <terminal.h>
#include <types.h>
#include <multibootInformations.h>
#include <lib/mem.h>
#include <memory.h>
#include <debug.h>
#include <synchronization/spinlock.h>

#include <log.h>

namespace Terminal
{
    static uint32 x = 0;
    static uint32 y = 0;
    static uint64 video_address = Memory::Virtual::getKernelVirtualAddress(0xB8000);
    static uint32 width = 80;
    static uint32 height = 25;
    static uint8 style = 0x07;

    void init()
    {
        MultibootInformations::FramebufferInfo *framebuffer = (MultibootInformations::FramebufferInfo *)MultibootInformations::getEntry(MultibootInformations::FRAMEBUFFER_INFO);
        if (framebuffer == nullptr)
            return;
        if (framebuffer->framebuffer_type != 2)
            return;
        video_address = Memory::Virtual::getKernelVirtualAddress(framebuffer->address);
        width = framebuffer->width;
        height = framebuffer->height;
        style = 0x07;
    }

    void drawChar(const char c, uint x, uint y)
    {
        assert(x < width);
        assert(y < height);
        *((unsigned short *)video_address + y * width + x) = c | style << 8;
    }

    char getChar(uint x, uint y)
    {
        assert(x < width);
        assert(y < height);
        return *((unsigned short *)video_address + y * width + x);
    }

    void printChar(const char c)
    {
        if (c == '\n')
        {
            x = 0;
            y++;
            return;
        }
        if (c == '\b')
        {
            if (x > 0)
                x--;
            else if (y > 0)
            {
                y--;
                x = width;
                char _c = getChar(--x, y);
                while (x && (_c == 0 || _c == ' '))
                    _c = getChar(--x, y);
            }
            drawChar(0, x, y);
            return;
        }
        if (y >= height)
        {
            y = height - 1;
            memmove((void *)video_address, (void *)(video_address + width * 2), (height - 1) * width * 2);
            memset((void *)(video_address + (height - 1) * width * 2), 0, width * 2);
        }
        drawChar(c, x, y);
        if (++x >= width)
        {
            x = 0;
            y++;
            if (y >= height)
            {
                y = height - 1;
                memmove((void *)video_address, (void *)(video_address + width * 2), (height - 1) * width * 2);
                memset((void *)(video_address + (height - 1) * width * 2), 0, width * 2);
            }
        }
    }

    void printInt(String &str, uint32 value)
    {
        if (value < 10)
        {
            str.push(48 + value);
            return;
        }
        printInt(str, value / 10);
        str.push(48 + (value % 10));
    }

    void printLong(String &str, uint64 value)
    {
        if (value < 10)
        {
            str.push(48 + value);
            return;
        }
        printLong(str, value / 10);
        str.push(48 + (value % 10));
    }

    void printHex(String &str, uint64 value)
    {
        if (value < 16)
        {
            if (value < 10)
                str.push(48 + value);
            else
                str.push(55 + value);
            return;
        }
        printHex(str, value / 16);
        if (value % 16 < 10)
            str.push(48 + (value % 16));
        else
            str.push(55 + (value % 16));
    }

    void printStr(String &str, const char *ptr)
    {
        while (*ptr)
            str.push(*ptr++);
    }

    void printStr(const char *str, uint length)
    {
        for (uint i = 0; i < length; i++)
            printChar(str[i]);
    }

    void printStr(const String &str)
    {
        for (uint i = 0; i < str.size(); i++)
            printChar(str[i]);
    }

    void _sprintf(String &str, const char *format, va_list &va)
    {
        while (*format)
        {
            if (*format != '%')
            {
                str.push(*format++);
                continue;
            }
            format++;

            switch (*format)
            {
            case 'i':
                printInt(str, (uint32)va_arg(va, uint32));
                break;

            case 'l':
                printLong(str, (uint64)va_arg(va, uint64));
                break;

            case 'x':
                str.push('0');
                str.push('x');
                printHex(str, (uint64)va_arg(va, uint64));
                break;

            case 's':
                printStr(str, (const char *)va_arg(va, const char *));
                break;

            case 'S':
                printStr(str, ((String *)va_arg(va, String *))->c_str());
                break;

            default:
                break;
            }

            format++;
        }
    }

    void kprintf(const char *format, ...)
    {
        va_list va;
        va_start(va, format);
        String str;
        _sprintf(str, format, va);
        printStr(str);
        va_end(va);
    }

    void sprintf(String &str, const char *format, ...)
    {
        va_list va;
        va_start(va, format);
        _sprintf(str, format, va);
        va_end(va);
    }

    namespace safe
    {
        void print(const char *str)
        {
            while (*str)
                printChar(*(str++));
        }

        void println(const char *str)
        {
            print(str);
            printChar('\n');
        }

        void _printHex(uint64 value)
        {
            if (value < 16)
            {
                if (value < 10)
                    printChar(48 + value);
                else
                    printChar(55 + value);
                return;
            }
            _printHex(value / 16);
            if (value % 16 < 10)
                printChar(48 + (value % 16));
            else
                printChar(55 + (value % 16));
        }

        void printHex(uint64 value)
        {
            print("0x");
            _printHex(value);
        }

        void printInt(uint64 value)
        {
            if (value < 10)
            {
                printChar(48 + value);
                return;
            }
            printInt(value / 10);
            printChar(48 + (value % 10));
        }

    }

} // namespace Terminal
