#pragma once

#include <terminal.h>
#include <types.h>

__attribute__((noreturn)) inline void _panic(const char *str, uint32 line, const char *file)
{
    Terminal::safe::print("Kernel panic: ");
    Terminal::safe::println(str);
    Terminal::safe::print(file);
    Terminal::safe::print(" line ");
    Terminal::safe::printInt(line);
halt:
    __asm__ volatile("cli\nhlt");
    goto halt;
}

#define panic(str) _panic(str, __LINE__, __FILE__);