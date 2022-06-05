#include <fs/fs.h>
#include <fs/drivers.h>
#include <log.h>
#include <lib/vector.h>
#include <errno.h>

namespace fs
{

    int64 Node::open(uint mode)
    {
        Log::warn("Trying to open base node");
        return -EINVAL;
    }

    int64 Node::close()
    {
        Log::warn("Trying to close base node");
        return -EINVAL;
    }

    int64 Node::read(uint64 offset, uint size, void *buffer)
    {
        Log::warn("Trying to read base node");
        return -EINVAL;
    }
    int64 Node::write(uint64 offset, uint size, void *buffer)
    {
        Log::warn("Trying to write base node");
        return -EINVAL;
    }
    int64 Node::readDir(uint offset, uint count, uint size, DirEntry *buffer)
    {
        Log::warn("Trying to readDir base node");
        return -EINVAL;
    }
    uint64 Node::size()
    {
        Log::warn("Trying to get base node size");
        return 0;
    }

    int64 Node::findEntry(const String &name, Node *&out)
    {
        Log::warn("Trying to find base node entry");
        return -EINVAL;
    }

    Vector<MountPoint> mountPoints;
    MountPoint rootMoutPoint;

    void init()
    {
        new (&mountPoints) Vector<MountPoint>();
    }

    bool isPathSubset(const Vector<String> &subset, const Vector<String> &total)
    {
        if (subset.size() > total.size())
            return false;

        for (uint i = 0; i < subset.size(); i++)
        {
            if (subset[i] != total[i])
                return false;
        }

        return true;
    }

    MountPoint *findMountPoint(const Vector<String> &pathComponents)
    {
        if (pathComponents.size() == 1 && pathComponents[0] == "/")
            return &rootMoutPoint;

        MountPoint *betterNode = nullptr;
        uint betterMatch = 0;
        for (MountPoint &mountPoint : mountPoints)
        {
            Vector<String> nodepathComponents = mountPoint.path.split('/');
            if (isPathSubset(nodepathComponents, pathComponents))
            {
                if (mountPoint.path.size() > betterMatch)
                {
                    betterNode = &mountPoint;
                    betterMatch = mountPoint.path.size();
                }
            }
        }
        return betterNode;
    }

    MountPoint *findMountPoint(const String &path)
    {
        Vector<String> pathComponents = path.split('/');
        return findMountPoint(pathComponents);
    }

    int mountNode(const String &path, Node *node)
    {
        MountPoint mountPoint = {.node = node, .path = path};
        mountPoints.push(mountPoint);
        if (path == "/")
            rootMoutPoint = mountPoint;
        Log::info("Mount %s", path.c_str());
        return 0;
    }

    int mount(const String &path, Node *device)
    {
        if (!device)
            return -ENODEV;
        FsDriver *driver = findFsDriver(device);
        if (!driver)
            return -ENODEV;

        return mountNode(path, driver->mount(device, path.slice(path.findLastIndex('/'), path.size())));
    }

    int64 resolvePath(const String &path, Node *&out)
    {
        if (path.empty())
            return -ENOENT;
        if (path[0] != '/')
            return -ENOENT;

        MountPoint *mountPoint = findMountPoint(path);
        if (!mountPoint)
            return -ENOENT;

        Vector<String> newPathComponents = path.slice(mountPoint->path.size(), path.size()).split('/');

        Node *currentNode = mountPoint->node;
        for (uint i = 0; i < newPathComponents.size(); i++)
        {
            String &component = newPathComponents[i];
            if (!(currentNode->type & NodeType::DIRECTORY))
                return -ENOTDIR;

            currentNode->open(OpenMode::READ);

            Node *old_node = currentNode;
            ERROR_CHECK(currentNode->findEntry(component, currentNode));
            old_node->close();
        }

        out = currentNode;
        return 0;
    }

} // namespace fs
