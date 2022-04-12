#pragma once
#include <types.h>

namespace Log
{
    void debug(const char *format, ...);
    void info(const char *format, ...);
    void warn(const char *format, ...);
    void raw(const char *str);
} // namespace Log
