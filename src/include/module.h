#pragma once

struct Module
{
    const char *name;
    int (*init)();
    void (*unload)();
};

#define DECLARE_MODULE(_name, _init, _unload) Module __module = {.name = _name, .init = _init, .unload = _unload};
