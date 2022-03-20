#include <drivers/diskDriver.h>
#include <fs/fs.h>
#include <lib/vector.h>

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

        class File : public FileNode
        {
            TarFileHeader *m_header;
            void *m_start;

        public:
            File(TarFileHeader *driver)
            {
                m_header = driver;
                m_start = (void *)(((uint64)m_header | 0x1FF) + 1);
            }

            int64 read(uint64 offset, uint64 size, void *buffer)
            {
                uint64 sizeToRead = getsize(m_header->size);
                if (size < sizeToRead)
                    sizeToRead = size;
                memcpy(buffer, (void *)((uint64)m_start + offset), sizeToRead);
                return sizeToRead;
            }
        };

        class RootNode : public MountNode
        {
        private:
            Vector<TarFileHeader *> files;

        public:
            RootNode(InitrdDriver *driver, const String &path) : MountNode(path)
            {
                uint64 i = 0;
                while (i < driver->getSize())
                {
                    TarFileHeader *file = (TarFileHeader *)(driver->buffer + i);
                    if (file->filename[0] == '\0')
                        break;

                    files.push(file);
                    uint64 size = getsize(file->size);
                    i += ((size / 512) + 1) * 512;
                    if (size % 512)
                        i += 512;
                }
            }

            Node *findEntry(const String &name)
            {
                for (TarFileHeader *file : files)
                {
                    if (name == file->filename)
                        return new File(file);
                }
                return nullptr;
            }
        };

        void init()
        {
            MultibootInformations::Module *initrd = MultibootInformations::findModule("initrd");
            assert(initrd);
            InitrdDriver *initrdDriver = new InitrdDriver((void *)Memory::Virtual::getKernelVirtualAddress(initrd->start), initrd->end - initrd->start);
            fs::mount("/initrd", fs::Initrd::createRootNode(initrdDriver, "/initrd"));
        }

        MountNode *createRootNode(InitrdDriver *driver, const String &path)
        {
            return new RootNode(driver, path);
        }
    } // namespace Tar

} // namespace fs
