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
        Log::info("Create process %i", process->id);
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
        addThread(thread);

        return process;
    }

    void destroyProcess(Process *process)
    {
        assert(process);
        CPU *cpu = getCurrentCPU();
        assert(process != cpu->currentProcess);
        assert(process->threads.empty());

        Memory::Virtual::destroyAddressSpace(&process->addressSpace);

        Log::info("Scheduler: destroyed process %i", process->id);
    }

} // namespace Scheduler
