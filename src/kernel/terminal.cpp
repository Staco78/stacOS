#include <terminal.h>
#include <stdarg.h>
#include <types.h>
#include <multibootInformations.h>
#include <lib/mem.h>
#include <memory.h>
#include <debug.h>

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

    void printChar(const char c)
    {
        if (y >= height)
        {
            y = height - 1;
            memcpy((void *)video_address, (void *)(video_address + width * 2), (height - 1) * width * 2);
            memset((void *)(video_address + (height - 1) * width * 2), 0, width * 2);
        }
        if (c == '\n')
        {
            x = 0;
            y++;
            return;
        }
        *((unsigned short *)video_address + y * width + x) = c | style << 8;
        if (++x >= width)
        {
            x = 0;
            y++;
        }
    }

    void printInt(uint32 value)
    {
        if (value < 10)
        {
            printChar(48 + value);
            return;
        }
        printInt(value / 10);
        printChar(48 + (value % 10));
    }

    void printLong(uint64 value)
    {
        if (value < 10)
        {
            printChar(48 + value);
            return;
        }
        printLong(value / 10);
        printChar(48 + (value % 10));
    }

    void printHex(uint64 value)
    {
        if (value < 16)
        {
            if (value < 10)
                printChar(48 + value);
            else
                printChar(55 + value);
            return;
        }
        printHex(value / 16);
        if (value % 16 < 10)
            printChar(48 + (value % 16));
        else
            printChar(55 + (value % 16));
    }

    void printStr(const char *str)
    {
        while (*str)
            printChar(*str++);
    }

    void kprintf(const char *format, ...)
    {
        va_list va;
        va_start(va, format);

        while (*format)
        {
            if (*format != '%')
            {
                printChar(*format++);
                continue;
            }
            format++;

            switch (*format)
            {
            case 'i':
                printInt((uint32)va_arg(va, uint32));
                break;

            case 'l':
                printLong((uint64)va_arg(va, uint64));
                break;

            case 'x':
                printChar('0');
                printChar('x');
                printHex((uint64)va_arg(va, uint64));
                break;

            case 's':
                printStr((const char *)va_arg(va, const char *));
                break;

            default:
                break;
            }

            format++;
        }

        va_end(va);
    }

    void printStr(const char *str, uint length)
    {
        for (uint i = 0; i < length; i++)
            printChar(str[i]);
    }
} // namespace Terminal
