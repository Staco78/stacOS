#pragma once
#include <types.h>
#include <lib/string.h>

namespace Serial
{
    void init();
    void print(const char c);
    void print(const char *str);
    void print(String &str);
} // namespace Serial
