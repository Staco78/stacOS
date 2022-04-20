#pragma once
#include <types.h>

namespace Log
{
    void debug(const char *format, ...);
    void info(const char *format, ...);
    void warn(const char *format, ...);
    void raw(const char *str);

    namespace safe
    {
        void print(const char *str);
        void println(const char *str);
        void printHex(uint64 value);
        void printInt(uint64 value);
    } // namespace safe
} // namespace Log
