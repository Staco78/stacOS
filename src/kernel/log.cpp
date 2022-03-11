#include <log.h>
#include <terminal.h>
#include <devices/serial.h>

#include <stdarg.h>

namespace Log
{
    void debug(const char *format, ...)
    {
        va_list va;
        va_start(va, format);
        String str;
        Terminal::_sprintf(str, format, va);
        str.push('\n');
        Serial::print(str);
        va_end(va);
    }
} // namespace Log
