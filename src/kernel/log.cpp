#include <log.h>
#include <terminal.h>
#include <devices/serial.h>

#include <stdarg.h>

namespace Log
{

    enum DebugLevel
    {
        Error = 1,
        Warn,
        Info,
        Debug
    };

    static constexpr DebugLevel actualDebugLevel = Debug;

    void debug(const char *format, ...)
    {
        if (actualDebugLevel >= Debug)
        {
            va_list va;
            va_start(va, format);
            String str;
            Terminal::_sprintf(str, format, va);
            str.push('\n');
            Serial::print(str);
            va_end(va);
        }
    }

    void warn(const char *format, ...)
    {
        if (actualDebugLevel >= Warn)
        {
            va_list va;
            va_start(va, format);
            String str("Warn: ", 6);
            Terminal::_sprintf(str, format, va);
            str.push('\n');
            Serial::print(str);
            va_end(va);
        }
    }

    void info(const char *format, ...)
    {
        if (actualDebugLevel >= Info)
        {
            va_list va;
            va_start(va, format);
            String str("Info: ", 6);
            Terminal::_sprintf(str, format, va);
            str.push('\n');
            Serial::print(str);
            va_end(va);
        }
    }

    void raw(const char *str)
    {
        Serial::print(str);
    }
} // namespace Log
