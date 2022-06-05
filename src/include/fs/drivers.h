#pragma once
#include <fs/fs.h>

namespace fs
{
    class FsDriver
    {
    public:
        virtual bool identify(Node *device) = 0;
        virtual Node* mount(Node *device, const String& name) = 0;
    };

    void registerFsDriver(FsDriver *driver);
    void unregisterFsDriver(FsDriver *driver);
    FsDriver *findFsDriver(Node *device);
} // namespace fs
