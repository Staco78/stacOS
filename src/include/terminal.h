#pragma once
#include <multibootInformations.h>
#include <lib/string.h>

#include <stdarg.h>

namespace Terminal
{
    void init();
    void drawChar(const char c, uint x, uint y);
    void sprintf(String &str, const char *format, ...);
    void _sprintf(String &str, const char *format, va_list &va);
    void kprintf(const char *format, ...);
    void printStr(const char *str, uint length);
} // namespace Terminal
