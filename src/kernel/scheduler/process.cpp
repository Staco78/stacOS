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
        process->threads.clear();
        process->fds.clear();

        Log::info("Create process %i", process->id);
        return process;
    }

    Process *loadProcess(const char *path)
    {
        fs::Node *file;
        assert(fs::resolvePath(path, file) >= 0);
        assert(file);
        assert(file->type & fs::FILE);

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

    uint Process::allocateFdEntry()
    {
        for (uint i = 0; i < fds.size(); i++)
        {
            if (!fds[i].node)
                return i;
        }

        fds.push({.node = nullptr, .mode = 0});
        return fds.size() - 1;
    }

} // namespace Scheduler
