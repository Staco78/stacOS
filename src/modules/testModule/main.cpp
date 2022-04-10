#include <terminal.h>
#include <module.h>

int init()
{
    Terminal::kprintf("Hello from module\n");
    return 0;
}

void unload()
{
    Terminal::kprintf("unload\n");
}

DECLARE_MODULE("testModule", init, unload);