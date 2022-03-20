#pragma once
#include <debug.h>
#include <lib/string.h>
#include <log.h>
#include <types.h>
#include <drivers/diskDriver.h>

namespace fs
{
    enum class NodeType
    {
        FILE = 1,
        DIRECTORY = 2,
    };

    struct NodeStat
    {
        uint64 size;
    };

    class Node
    {
    public:
        NodeType type;
        Node *parent = nullptr;
        String name;

        virtual bool isFile();
        virtual bool isDirectory();

        virtual void stat(NodeStat *buf);
    };

    class FileNode : public Node
    {
    public:
        virtual int64 read(uint64 offset, uint64 size, void *buffer);
        bool isFile();
    };

    class DirectoryNode : public Node
    {
    public:
        virtual Node *findEntry(const String &name) = 0;
        bool isDirectory();
    };

    class MountNode : public DirectoryNode
    {
    public:
        String path;
        MountNode(const String &path)
        {
            MountNode::path = path;
        }
    };

    MountNode *findMountPoint(const String &path);
    void mount(const String &path, MountNode *node);
    Node *resolvePath(const String &path);

    namespace Initrd
    {
        void init();
        MountNode *createRootNode(InitrdDriver *driver, const String &path);
    } // namespace Tar

} // namespace fs
