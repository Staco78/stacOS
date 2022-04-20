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

    namespace safe
    {
        void print(const char *str)
        {
            while (*str)
                Serial::print(*(str++));
        }

        void println(const char *str)
        {
            print(str);
            Serial::print('\n');
        }

        void _printHex(uint64 value)
        {
            if (value < 16)
            {
                if (value < 10)
                    Serial::print(48 + value);
                else
                    Serial::print(55 + value);
                return;
            }
            _printHex(value / 16);
            if (value % 16 < 10)
                Serial::print(48 + (value % 16));
            else
                Serial::print(55 + (value % 16));
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
                Serial::print(48 + value);
                return;
            }
            printInt(value / 10);
            Serial::print(48 + (value % 10));
        }
    }

} // namespace Log
