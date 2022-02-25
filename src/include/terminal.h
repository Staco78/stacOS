#pragma once
#include <multibootInformations.h>

namespace Terminal
{
    void init();
    void kprintf(const char *format, ...);
    void printStr(const char *str, uint length);
} // namespace Terminal
