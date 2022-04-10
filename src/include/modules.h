#pragma once
#include <fs/fs.h>
#include <module.h>

namespace Modules
{
    struct LoadedModule
    {
        Module* data;
        uint64 address;
        uint size;
    };

    void init();
    bool loadModule(fs::FileNode *file);
} // namespace Modules
