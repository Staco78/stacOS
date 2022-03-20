#include <fs/fs.h>
#include <log.h>
#include <lib/vector.h>

namespace fs
{

    bool Node::isFile() { return false; }
    bool Node::isDirectory() { return false; }

    bool FileNode::isFile() { return true; }
    int64 FileNode::read(uint64 offset, uint64 size, void *buffer)
    {
        Log::warn("Trying to read base fileNode");
        return -1;
    }

    bool DirectoryNode::isDirectory() { return true; }
    Node *DirectoryNode::findEntry(const String &name) { return nullptr; }

    Vector<MountNode *> mountPoints;
    MountNode *rootMoutPoint = nullptr;

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

    MountNode *findMountPoint(const Vector<String> &pathComponents)
    {
        if (pathComponents.size() == 1 && pathComponents[0] == "/")
            return rootMoutPoint;

        MountNode *betterNode = nullptr;
        uint betterMatch = 0;
        for (MountNode *node : mountPoints)
        {
            Vector<String> nodepathComponents = node->path.split('/');
            if (isPathSubset(nodepathComponents, pathComponents))
            {
                if (node->path.size() > betterMatch)
                {
                    betterNode = node;
                    betterMatch = node->path.size();
                }
            }
        }
        return betterNode;
    }

    MountNode *findMountPoint(const String &path)
    {
        Vector<String> pathComponents = path.split('/');
        return findMountPoint(pathComponents);
    }

    void mount(const String &path, MountNode *node)
    {
        mountPoints.push(node);
        if (path == "/")
            rootMoutPoint = node;
        Log::info("Mount %s", path.c_str());
    }

    Node *resolvePath(const String &path)
    {
        assert(!path.empty());
        assert(path[0] == '/');

        MountNode *mountPoint = findMountPoint(path);
        assert(mountPoint);

        Vector<String> newPathComponents = path.slice(mountPoint->path.size(), path.size()).split('/');

        Node *currentNode = mountPoint;
        for (uint i = 0; i < newPathComponents.size(); i++)
        {
            String &component = newPathComponents[i];
            if (!currentNode->isDirectory())
                return nullptr;

            DirectoryNode *dir = (DirectoryNode *)currentNode;
            currentNode = dir->findEntry(component);
            if (!currentNode)
                return nullptr;
        }

        return currentNode;
    }

} // namespace fs
