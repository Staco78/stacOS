#pragma once

#include <terminal.h>
#include <types.h>

__attribute__((noreturn)) inline void _panic(const char *str, uint32 line, const char *file)
{
    Terminal::kprintf("Kernel panic: %s \n%s line %i", str, file, line);
halt:
    __asm__ volatile("cli\nhlt");
    goto halt;
}

#define panic(str) _panic(str, __LINE__, __FILE__);