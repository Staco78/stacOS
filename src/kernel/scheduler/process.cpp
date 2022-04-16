#include <scheduler.h>
#include <memory.h>
#include <fs/fs.h>
#include <interrupts.h>

namespace Scheduler
{

    pid getNextProcessId()
    {
        static pid id = 1;
        return id++;
    }

    Process *createProcess(const char *name)
    {
        Process *process = new Process();
        process->name = name;
        process->id = getNextProcessId();
        process->addressSpace = Memory::Virtual::createAddressSpace();
        return process;
    }

    Process *loadProcess(const char *path)
    {
        fs::Node *node = fs::resolvePath(path);
        assert(node);
        assert(node->isFile());
        fs::FileNode *file = (fs::FileNode *)node;

        Process *process = createProcess(path);
        ELF::loadExecutable(process, file);

        assert(process->threads.size() == 0);

        Thread *thread = createThread(process);
        assert(thread);
        process->threads.push(thread);
        getCurrentCPU()->threads.lock();
        getCurrentCPU()->threads.push(thread);
        getCurrentCPU()->threads.unlock();

        return process;
    }
} // namespace Scheduler
