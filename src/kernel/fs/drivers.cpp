#include <fs/drivers.h>
#include <lib/vector.h>
#include <lib/synchronized.h>

namespace fs
{

    Synchronized<Vector<FsDriver *>> drivers;

    void initDrivers()
    {
        new (&drivers) Synchronized<Vector<FsDriver *>>();
    }

    void registerFsDriver(FsDriver *driver)
    {
        drivers.lock();
        drivers.push(driver);
        drivers.unlock();
    }

    void unregisterFsDriver(FsDriver *driver)
    {
        drivers.lock();
        drivers.remove(driver);
        drivers.unlock();
    }

    FsDriver *findFsDriver(Node *device)
    {
        assert(device);
        FsDriver **ptr = drivers.find([device](FsDriver *d)
                                      { return d->identify(device); });
        if (!ptr)
            return nullptr;
        return *ptr;
    }
} // namespace fs
