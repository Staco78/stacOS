#include <fs/fs.h>
#include <lib/vector.h>
#include <errno.h>

namespace fs
{

    namespace Initrd
    {
        struct TarFileHeader
        {
            char filename[100];
            uint64 mode;
            uint64 uid;
            uint64 gid;
            char size[12];
            char mtime[12];
            uint64 checksum;
            uint8 typeflag;

            // custom
            Node *node;
        } __attribute__((packed));

        uint64 getsize(const char *in)
        {

            uint64 size = 0;
            uint64 j;
            uint64 count = 1;

            for (j = 11; j > 0; j--, count *= 8)
                size += ((in[j - 1] - '0') * count);

            return size;
        }

        class File : public Node
        {
            TarFileHeader *m_header;
            void *m_start;
            uint64 m_size;

        public:
            File(TarFileHeader *header)
            {
                name = header->filename;
                type = FILE;
                m_header = header;
                m_start = (void *)(((uint64)m_header | 0x1FF) + 1);
                m_size = getsize(header->size);
            }

            int64 read(uint64 offset, uint size, void *buffer)
            {
                uint64 sizeToRead = m_size;
                if (size < sizeToRead)
                    sizeToRead = size;
                memcpy(buffer, (void *)((uint64)m_start + offset), sizeToRead);
                return sizeToRead;
            }

            uint64 size()
            {
                return m_size;
            }

            int64 open(uint mode)
            {
                if (mode & OpenMode::WRITE)
                    return -EROFS;
                return 0;
            }

            int64 close()
            {
                return 0;
            }
        };

        class RootNode : public Node
        {
        private:
            Vector<TarFileHeader *> files;

        public:
            RootNode(void *buffer, uint64 size, const String &name)
            {
                Node::name = name;
                type = DIRECTORY;
                uint64 i = 0;
                while (i < size)
                {
                    TarFileHeader *file = (TarFileHeader *)((uint64)buffer + i);
                    if (file->filename[0] == '\0')
                        break;

                    files.push(file);
                    uint64 size = getsize(file->size);
                    i += ((size / 512) + 1) * 512;
                    if (size % 512)
                        i += 512;
                }
            }

            int64 findEntry(const String &name, Node *&out)
            {
                for (TarFileHeader *file : files)
                {
                    if (name == file->filename)
                    {
                        if (!file->node)
                            file->node = new File(file);
                        out = file->node;
                        return 0;
                    }
                }
                return -ENOENT;
            }

            int64 open(uint mode)
            {
                if (mode & OpenMode::WRITE)
                    return -EROFS;
                return 0;
            }

            int64 close()
            {
                return 0;
            }
        };

        void init()
        {
            MultibootInformations::Module *initrd = MultibootInformations::findModule("initrd");
            assert(initrd);
            fs::mountNode("/initrd", new RootNode((void *)Memory::Virtual::getKernelVirtualAddress(initrd->start), initrd->end - initrd->start, "initrd"));
        }

    } // namespace Tar

} // namespace fs
