#include <symbols.h>
#include <log.h>
#include <fs/fs.h>
#include <lib/hashMap.h>

namespace KernelSymbols
{

    HashMap<String, uint64> map;

    void install()
    {
        fs::Node *file;
        assert(fs::resolvePath("/initrd/symbols", file) >= 0);
        assert(file);
        uint64 size = file->size();
        char *buf = (char *)kmalloc(size);
        assert(buf);
        assert(file->read(0, size, buf) > 0);

        new (&map) HashMap<String, uint64>(*(uint32 *)buf);

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
        if (!value)
        {
            Log::warn("Symbol %s not found", name.c_str());
            return 0;
        }
        assert(value);
        return *value;
    }
} // namespace Symbols
