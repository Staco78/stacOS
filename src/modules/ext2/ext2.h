#pragma once
#include <fs/fs.h>
#include <types.h>
#include <lib/hashMap.h>

#define MAGIC 0xEF53
#define DEFAULT_ROOT_INODE 2

#define CACHE_SIZE 100

enum VolumeError
{
    None = 0,
    InvalidDeviceError = 1,
    DeviceError = 2,
    FsError = 3,
    NotSupportedFeature = 4
};

namespace InodeType
{
    enum
    {
        FIFO = 1,
        Character = 2,
        Directory = 4,
        Block = 6,
        File = 8,
        Link = 0xA,
        Socket = 0xC
    };
} // namespace InodeType

struct Superblock
{
    uint32 inodeCount;     // Number of inodes (used + free) in the file system
    uint32 blockCount;     // Number of blocks (used + free + reserved) in the filesystem
    uint32 resvBlockCount; // Number of blocks reserved for the superuser
    uint32 freeBlockCount; // Total number of free blocks (including superuser reserved)
    uint32 freeInodeCount; // Number of free inodes
    uint32 firstDataBlock; // First datablock / Block containing this superblock (0 if block size>1KB as the
                           // superblock is ALWAYS at 1KB)
    uint32 logBlockSize;   // Block size is equal to (1024 << logBlockSize)
    uint32 logFragSize;    // Fragment size is equal to (1024 << logFragSize)
    uint32 blocksPerGroup; // Number of blocks per group
    uint32 fragsPerGroup;  // Number of fragments per group also used to determine block bitmap size
    uint32 inodesPerGroup; // Number of inodes per group
    uint32 mountTime;      // UNIX timestamp of the last time the filesystem was mounted
    uint32 writeTime;      // UNIX timestamp of the last time the filesystem was written to
    uint16 mntCount;       // How many times the filesystem has been mounted since it was last verified
    uint16 maxMntCount;    // The amount of times the filesystem should be mounted before a full check
    uint16 magic;          // Filesystem  Identifier (equal to 0xEF53)
    uint16 state;          // When mounted set to 2, on unmount 1. On mount if equal to
                           // 2 then the fs was not cleanly unmounted and may contain errors
    uint16 errors;         // What should the driver do when an error is detected?
    uint16 minorRevLevel;  // Minor revision level
    uint32 lastCheck;      // UNIX timestamp of the last filesystem check
    uint32 checkInterval;  // Maximum UNIX time interval allowed between filesystem checks
    uint32 creatorOS;      // ID of the OS that created the fs
    uint32 revLevel;       // Revision level
    uint16 resUID;         // Default user ID for reserved blocks
    uint16 resGID;         // Default group ID for reserved blocks

    // ext
    uint32 firstInode;        // First usable inode
    uint16 inodeSize;         // Size of the inode structure
    uint16 blockGroupNum;     // Inode structure size (always 128 in rev 0)
    uint32 featuresCompat;    // Compatible Features bitmask
    uint32 featuresIncompat;  // Incompatible features bitmask (Do not mount)
    uint32 featuresRoCompat;  // Features incompatible when writing bitmask (Mount as readonly)
    uint8 uuid[16];           // volume UUID
    char volumeName[16];      // Volume Name
    char lastMounted[64];     // Path where the filesystem was last mounted
    uint32 algorithmBitmap;   // Indicates compression algorithm
    uint8 preallocatedBlocks; // Blocks to preallocate when a file is created
    uint8 preallocdDirBlocks; // Blocks to preallocate when a directory is created
    uint16 align;
} __attribute__((packed));

struct BlockGroupDescriptor
{
    uint32 blockUsageAddress;
    uint32 inodeUsageAddress;
    uint32 inodeTableAddress;
    uint16 freeBlocksCount;
    uint16 freeInodesCount;
    uint16 directoriesCount;
    uint8 unused[14];
} __attribute__((packed));

struct Inode
{
    struct
    {
        uint16 permissions : 12;
        uint16 type : 4;
    };
    uint16 uid;
    uint32 lowerSize;
    uint32 accessTime;
    uint32 creationTime;
    uint32 modificationTime;
    uint32 deletionTime;
    uint16 gid;
    uint16 linkCount;
    uint32 blocksCount;
    uint32 flags;
    uint32 os1;
    uint32 blocks[15];
    uint32 generation;
    uint32 fileACL;
    union
    {
        uint32 upperSize;
        uint32 dirACL;
    };
    uint32 fragmentAddress;
    uint8 os2[12];
} __attribute__((packed));

struct DirectoryEntry
{
    uint32 inode;
    uint16 size;
    uint8 nameLengthLow;
    union
    {
        uint8 nameLengthHigh;
        uint8 type;
    };
    char name[];
} __attribute__((packed));

class Ext2Node;

class Volume
{
public:
    Volume(fs::Node *device, const String &mountName, bool readonly);
    int64 readInode(uint32 index, Inode *inode);
    int64 readBlock(uint index, void *buffer);
    int64 readInodeData(const Inode &inode, uint64 offset, uint size, void *buffer);

    Superblock super;

    BlockGroupDescriptor *blockGroups;

    uint32 blockSize;
    uint32 inodeSize;
    uint32 blockGroupsCount;

    bool readonly = false;

    fs::Node *rootNode;

    VolumeError err = None;

    fs::Node *device;

    HashMap<uint32, Ext2Node *> inodesCache;
};

Ext2Node *cacheOrCreateNode(Volume *volume, uint32 inodeIndex, const String &name);

class Ext2Node : public fs::Node
{
public:
    Ext2Node(Volume *volume, uint32 inodeIndex, const String &name);
    int64 open(uint mode) final;
    int64 close() final;
    int64 readDir(uint offset, uint count, uint size, fs::DirEntry *buffer) final;
    int64 findEntry(const String &name, fs::Node *&out) final;
    int64 read(uint64 offset, uint size, void *buffer) final;

private:
    Volume *m_volume;
    Inode m_inode;
    uint32 m_inodeIndex;
};
