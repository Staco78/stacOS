#include <fs/fs.h>
#include <scheduler.h>
#include <errno.h>

extern "C"
{
    int64 sysOpen(const char *path, uint mode)
    {
        if (!Memory::checkStrAccess(path))
            return -EFAULT;

        Scheduler::CPU *cpu = Scheduler::getCurrentCPU();
        Scheduler::Process *process = cpu->currentProcess;

        fs::Node *node;
        ERROR_CHECK(fs::resolvePath(path, node));
        ERROR_CHECK(node->open(mode));

        uint fdIndex = process->allocateFdEntry();
        Scheduler::FdEntry &fd = process->fds[fdIndex];
        fd.node = node;
        fd.mode = mode;

        return fdIndex;
    }

    int64 sysWrite(uint32 fd, void *buffer, uint size, uint64 offset)
    {
        if (!Memory::checkAccess((uint64)buffer, size))
            return -EFAULT;

        Scheduler::CPU *cpu = Scheduler::getCurrentCPU();
        Scheduler::Process *process = cpu->currentProcess;

        if (fd == 1 || fd == 2)
        {
            Terminal::printStr((char *)buffer, size);
            return size;
        }

        if (fd >= process->fds.size())
            return -EBADF;

        Scheduler::FdEntry entry = process->fds[fd];
        if (!entry.node)
            return -EBADF;

        if (!(entry.mode & fs::OpenMode::WRITE))
            return -EACCES;

        return entry.node->write(offset, size, buffer);
    }

    int64 sysRead(uint32 fd, void *buffer, uint size, uint64 offset)
    {
        if (!Memory::checkAccess((uint64)buffer, size))
            return -EFAULT;

        Scheduler::CPU *cpu = Scheduler::getCurrentCPU();
        Scheduler::Process *process = cpu->currentProcess;

        if (fd >= process->fds.size())
            return -EBADF;

        Scheduler::FdEntry entry = process->fds[fd];
        if (!entry.node)
            return -EBADF;

        if (!(entry.mode & fs::OpenMode::READ))
            return -EACCES;

        return entry.node->read(offset, size, buffer);
    }
}