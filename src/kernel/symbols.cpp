#include <symbols.h>
#include <log.h>
#include <fs/fs.h>
#include <lib/hashMap.h>

namespace KernelSymbols
{

    StringHashMap<uint64> map;

    void install()
    {
        fs::Node *node = fs::resolvePath("/initrd/symbols");
        assert(node);
        assert(node->isFile());
        fs::FileNode *file = (fs::FileNode *)node;
        uint64 size = file->getSize();
        char *buf = (char *)kmalloc(size);
        file->read(0, size, buf);

        new (&map) StringHashMap<uint64>(*(uint32 *)buf);

        uint i = 4;
        while (i < size)
        {
            map.set(String(buf + i + 8), *(uint64 *)((uint64)buf + i));
            i += 8;
            i += strlen(buf + i);
            i++;
        }

        kfree(buf);
    }

    uint64 get(const String &name)
    {
        uint64 *value = map.get(name);
        assert(value);
        return *value;
    }
} // namespace Symbols
