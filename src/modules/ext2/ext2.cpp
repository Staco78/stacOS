#include <module.h>
#include <fs/drivers.h>
#include <errno.h>
#include <lib/math.h>
#include "ext2.h"

using namespace fs;

uint fsTypeFromExtType(uint16 t)
{
    switch (t)
    {
    case InodeType::Directory:
        return fs::DIRECTORY;

    default:
        return fs::FILE;
    }
}

Volume::Volume(Node *device, const String &mountName, bool readonly) : inodesCache(CACHE_SIZE)
{
    Volume::device = device;
    Volume::readonly = readonly;
    if (!device)
    {
        err = InvalidDeviceError;
        return;
    }

    if (device->size() < 2048)
    {
        err = InvalidDeviceError;
        return;
    }

    int64 r = device->read(1024, sizeof(Superblock), &super);
    if (r < (int64)sizeof(Superblock))
    {
        err = DeviceError;
        return;
    }

    if (super.magic != MAGIC)
    {
        err = InvalidDeviceError;
        return;
    }

    if (super.state != 1)
    {
        switch (super.errors)
        {
        case 1:
            // ignore
            break;

        case 2:
            Volume::readonly = true;
            break;

        default:
            err = FsError;
            return;
        }
    }

    // features detection
    if (super.revLevel > 0)
    {
        if (super.featuresIncompat != 0)
        {
            err = NotSupportedFeature;
            return;
        }

        if (super.featuresRoCompat != 0)
        {
            Volume::readonly = true;
        }
    }

    blockSize = 1024U << super.logBlockSize;
    inodeSize = super.revLevel > 0 ? super.inodeSize : 128;

    if (inodeSize != sizeof(Inode))
        Log::warn("Ext2: Inode size mismatch");

    blockGroupsCount = (super.blockCount / super.blocksPerGroup) + ((super.blockCount % super.blocksPerGroup) != 0);

    uint blockGroupsTableAddress = blockSize == 1024 ? 2 : 1;

    blockGroups = (BlockGroupDescriptor *)kmalloc(sizeof(BlockGroupDescriptor) * blockGroupsCount);
    device->read(blockGroupsTableAddress * blockSize, blockGroupsCount * sizeof(BlockGroupDescriptor), blockGroups);

    rootNode = cacheOrCreateNode(this, 2, mountName);
}

int64 Volume::readInode(uint32 index, Inode *inode)
{
    uint blockGroupIndex = (index - 1) / super.inodesPerGroup;
    BlockGroupDescriptor &blockGroup = blockGroups[blockGroupIndex];
    uint32 localIndex = (index - 1) % super.inodesPerGroup;
    uint32 offset = blockGroup.inodeTableAddress * blockSize + localIndex * inodeSize;
    if (device->read(offset, sizeof(Inode), inode) < (int64)sizeof(Inode))
        return -EIO;
    return 0;
}

int64 Volume::readBlock(uint index, void *buffer)
{
    assert(buffer);
    int64 r = device->read(index * blockSize, blockSize, buffer);
    if (r < blockSize)
        return -EIO;

    return r;
}

int64 Volume::readInodeData(const Inode &inode, uint64 offset, uint size, void *buffer)
{
    uint blockIndexStart = offset / blockSize;
    uint blockIndexEnd = (offset + size) / blockSize + (size % blockSize != 0);

    assert(blockIndexEnd >= blockIndexStart);

    uint64 end = offset + size;
    uint bufferOffset = 0;

    auto readBlock = [this, buffer, offset, end, bufferOffset](uint index) -> int64
    {
        uint readOffset = offset % blockSize;
        uint readSize = min(end - offset, (uint64)blockSize);

        if (readOffset + readSize > blockSize)
            readSize = blockSize - readOffset;

        assert(readOffset + readSize <= blockSize);

        if (readSize == 0)
            return 0;

        if (readOffset == 0 && readSize == blockSize)
        {
            return Volume::readBlock(index, (uint8 *)buffer + bufferOffset);
        }

        uint8 blockBuffer[blockSize];
        ERROR_CHECK(Volume::readBlock(index, blockBuffer));

        memcpy((uint8 *)buffer + bufferOffset, blockBuffer + readOffset, readSize);
        return readSize;
    };

    uint i = blockIndexStart;
    while (i < 12)
    {
        if (i >= blockIndexEnd)
            return bufferOffset;

        int64 r = ERROR_CHECK(readBlock(inode.blocks[i]));

        bufferOffset += r;
        offset += r;

        i++;
    }

    if (i >= blockIndexEnd)
        return bufferOffset;

    // indirect block pointers
    {
        uint32 pointers[blockSize / sizeof(uint32)];
        Volume::readBlock(inode.blocks[12], pointers);

        while (i < (blockSize / sizeof(uint32) + 12))
        {
            if (i >= blockIndexEnd)
                return bufferOffset;

            int64 r = ERROR_CHECK(readBlock(pointers[i - 12]));
            bufferOffset += r;
            offset += r;
            i++;
        }
    }

    if (i >= blockIndexEnd)
        return bufferOffset;

    assert(!"Double indirect pointers are not supported yet");
}

Ext2Node *cacheOrCreateNode(Volume *volume, uint32 inodeIndex, const String &name)
{
    Ext2Node **node = volume->inodesCache.get(inodeIndex);
    if (node)
        return *node;
    Ext2Node *node2 = new Ext2Node(volume, inodeIndex, name);
    Log::info("Ext2: Created node %s (inode %i)", name.c_str(), inodeIndex);
    volume->inodesCache.set(inodeIndex, node2);
    return node2;
}

Ext2Node::Ext2Node(Volume *volume, uint32 inodeIndex, const String &name)
{
    m_volume = volume;
    m_inodeIndex = inodeIndex;
    m_volume->readInode(inodeIndex, &m_inode);
    Node::name = name;
    type = fsTypeFromExtType(m_inode.type);
}

int64 Ext2Node::open(uint mode)
{
    if (mode & OpenMode::WRITE && m_volume->readonly)
        return -EROFS;

    Log::info("Ext2: open %s (inode %i)", name.c_str(), m_inodeIndex);

    refCount++;
    return 0;
}

int64 Ext2Node::close()
{
    assert(refCount > 0);
    refCount--;
    if (refCount == 0)
    {
        Log::info("Ext2: Delete node %s (inode %i)", name.c_str(), m_inodeIndex);
        m_volume->inodesCache.remove(m_inodeIndex);
        delete this;
    }
    return 0;
}

int64 Ext2Node::readDir(uint offset, uint count, uint size, DirEntry *buffer)
{
    assert(buffer);

    if (!(type & fs::DIRECTORY))
        return -ENOTDIR;

    uint8 data[m_volume->blockSize];
    Terminal::kprintf("%x\n", m_volume->readInodeData(m_inode, 0, m_volume->blockSize, data));

    uint i = 0;
    uint c = 0;
    uint fileOff = 0;
    uint off = 0;
    uint bufferOff = 0;
    while (i < offset + count)
    {
        DirectoryEntry *entry = (DirectoryEntry *)(data + off);
        if (i >= offset)
        {
            uint entrySize = sizeof(DirEntry) + entry->nameLengthLow + 1;
            if (bufferOff + entrySize >= size)
            {
                return c;
            }
            DirEntry *bufferEntry = (DirEntry *)((uint64)buffer + bufferOff);
            bufferEntry->type = fsTypeFromExtType(entry->type);
            bufferEntry->offset = entrySize;
            memcpy(&bufferEntry->name, entry->name, entry->nameLengthLow);
            *(char *)(bufferEntry->name + entry->nameLengthLow) = 0;
            bufferOff += entrySize;
            c++;
        }
        off += entry->size;
        i++;

        if (off >= m_volume->blockSize)
        {
            fileOff += m_volume->blockSize;
            if (fileOff + m_volume->blockSize > m_inode.lowerSize)
                return c;
            m_volume->readInodeData(m_inode, fileOff, m_volume->blockSize, data);
            off -= m_volume->blockSize;
        }
    }

    return c;
}

int64 Ext2Node::findEntry(const String &name, Node *&out)
{
    if (!(type & fs::DIRECTORY))
        return -ENOTDIR;

    uint64 offset = 0;
    uint64 totalOffset = 0;
    uint64 fileOffset = 0;
    uint8 buffer[m_volume->blockSize];

    ERROR_CHECK(m_volume->readInodeData(m_inode, 0, m_volume->blockSize, buffer));

    while (totalOffset < m_inode.lowerSize)
    {
        DirectoryEntry *entry = (DirectoryEntry *)(buffer + offset);
        String entryName(entry->name, entry->nameLengthLow);
        if (name == entryName)
        {
            out = cacheOrCreateNode(m_volume, entry->inode, entryName);
            return 0;
        }
        offset += entry->size;
        totalOffset += entry->size;

        if (offset >= m_volume->blockSize)
        {
            ERROR_CHECK(m_volume->readInodeData(m_inode, fileOffset, m_volume->blockSize, buffer));

            offset -= m_volume->blockSize;
            fileOffset += m_volume->blockSize;
        }
    }

    return -ENOENT;
}

int64 Ext2Node::read(uint64 offset, uint size, void *buffer)
{
    return m_volume->readInodeData(m_inode, offset, size, buffer);
}

class Ext2 : FsDriver
{
public:
    void init()
    {
        registerFsDriver(this);
    }

    virtual ~Ext2()
    {
        unregisterFsDriver(this);
    }

    bool identify(Node *device)
    {
        if (!device)
            return false;
        if (device->size() < 2048)
            return false;

        Superblock superblock;
        int64 r = device->read(1024, sizeof(Superblock), &superblock);
        if (r < (int64)sizeof(Superblock))
            return false;

        if (superblock.magic != MAGIC)
            return false;

        if (superblock.revLevel < 1)
            return true;

        return superblock.featuresIncompat == 0;
    }

    Node *mount(Node *device, const String &name)
    {
        if (!device)
            return nullptr;

        Volume *vol = new Volume(device, name, false);
        if (vol->err)
            return nullptr;

        return vol->rootNode;
    }
};

Ext2 instance;

int init()
{
    instance.init();
    return 0;
}

void unload()
{
    instance.~Ext2();
}

DECLARE_MODULE("ext2", init, unload);