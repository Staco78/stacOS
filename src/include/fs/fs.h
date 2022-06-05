#pragma once
#include <debug.h>
#include <lib/string.h>
#include <log.h>
#include <types.h>

#define ERROR_CHECK(func) ({int64 e = func; if (e < 0) return e; e; })

namespace fs
{
    enum NodeType
    {
        FILE = 1,
        DIRECTORY = 2,
    };

    namespace OpenMode
    {
        enum
        {
            READ = 1,
            WRITE = 2
        };
    } // namespace OpenMode

    struct DirEntry
    {
        uint offset; // offset to next entry
        uint type;   // file type
        char name[]; // null-terminated file name
    };

    class Node
    {
    public:
        virtual ~Node() = default;

        uint type;
        String name;

        virtual int64 open(uint mode);
        virtual int64 close();
        virtual int64 read(uint64 offset, uint size, void *buffer);
        virtual int64 write(uint64 offset, uint size, void *buffer);
        virtual int64 readDir(uint offset, uint count, uint size, DirEntry *buffer);
        virtual int64 findEntry(const String &name, Node *&out);
        virtual uint64 size();

    protected:
        uint refCount = 0;
    };

    struct MountPoint
    {
        Node *node;
        String path;
    };

    void init();
    MountPoint *findMountPoint(const String &path);
    int mountNode(const String &path, Node *node);
    int mount(const String &path, Node *device);
    int64 resolvePath(const String &path, Node *&out);

    namespace DeviceManager
    {
        void init();
        void registerDevice(Node *node);
    }

    namespace Initrd
    {
        void init();
    } // namespace Tar

} // namespace fs
