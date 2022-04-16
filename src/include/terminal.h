#pragma once
#include <multibootInformations.h>
#include <lib/string.h>

#include <stdarg.h>

namespace Terminal
{
    void init();
    void drawChar(const char c, uint x, uint y);
    void printChar(const char c);
    void sprintf(String &str, const char *format, ...);
    void _sprintf(String &str, const char *format, va_list &va);
    void kprintf(const char *format, ...);
    void printStr(const char *str, uint length);

    namespace safe
    {
        void print(const char *str);
        void println(const char *str);
        void printHex(uint64 value);
        void printInt(uint64 value);
    } // namespace safe

} // namespace Terminal
