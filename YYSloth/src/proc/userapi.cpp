#include <fs/vfs.hpp>
#include <memory/cow.hpp>
#include <memory/kvmmngr.hpp>
#include <memory/msecurity.hpp>
#include <memory/vmbase.hpp>
#include <proc/elf.hpp>
#include <proc/proc.hpp>
#include <proc/stackpool.hpp>
#include <proc/userapi.hpp>
#include <proc/usermode.hpp>

namespace proc {
    [[noreturn]] void YY_ExitProcess() {
        core::log("YY_ExitProcess();\n\r");
        proc::ProcessManager::exit();
    }
    extern "C" int64_t YY_ConsoleWrite(char *location, uint64_t size) {
        core::log("YY_ConsoleWrite(%p, %llu);\n\r", location, size);
        if (size > YY_ConsoleOperationsSizeLimit) {
            return -1;
        }
        if (!memory::virtualRangeConditionCheck((memory::vaddr_t)location, size,
                                                true, false, false)) {
            return -1;
        }
        core::putsn(location, size);
        return 1;
    }
    extern "C" int64_t YY_GetSystemInfo(YY_SystemInfo *info) {
        if (!memory::virtualRangeConditionCheck((memory::vaddr_t)info,
                                                sizeof(YY_SystemInfo), true,
                                                true, false)) {
            return 0;
        }
        core::log("YY_GetSystemInfo(%p);\n\r", info);
        memset(info, sizeof(YY_SystemInfo), '\0');

        static const char kernelName[] = "YYSloth";
        static const char kernelRelease[] = "0.0.1";
        static const char kernelVersion[] = "#1-YayOS " __TIME__ " " __DATE__;
        static const char arch[] = "amd64";
        static const char operatingSystem[] = "YayOS";
        memcpy(&(info->kernelName), kernelName, sizeof(kernelName));
        memcpy(&(info->kernelRelease), kernelRelease, sizeof(kernelRelease));
        memcpy(&(info->kernelVersion), kernelVersion, sizeof(kernelVersion));
        memcpy(&(info->machine), arch, sizeof(arch));
        memcpy(&(info->processor), arch, sizeof(arch));
        memcpy(&(info->hardwarePlatform), arch, sizeof(arch));
        memcpy(&(info->operatingSystem), operatingSystem,
               sizeof(operatingSystem));
        return 1;
    }
    extern "C" void sysForkWithFrame(SchedulerIntFrame *frame) {
        core::log("YY_DuplicateProcess();\n\r");
        pid_t newProcessID = ProcessManager::newProcess();
        if (newProcessID == PID_MAX) {
            frame->rax = (uint64_t)(-1);
            return;
        }
        Process *newProc = ProcessManager::getProcessData(newProcessID);
        Process *currentProc = ProcessManager::getRunningProcess();
        if (!newProc->setup(false)) {
            ProcessManager::freePid(newProcessID);
            frame->rax = (uint64_t)(-1);
            return;
        }
        if ((newProc->usralloc = currentProc->usralloc->copy()) == nullptr) {
            newProc->cleanup();
            StackPool::pushStack(newProc->kernelStackBase);
            frame->rax = (uint64_t)(-1);
            return;
        }
        newProc->pid = newProcessID;
        newProc->ppid = currentProc->pid;
        newProc->dead = 0;
        frame->rax = 0;
        newProc->state.generalRegs.copyFrom(frame);
        newProc->state.generalRegs.cr3 = memory::CoW::clonePageTable();
        if (newProc->state.generalRegs.cr3 == 0) {
            newProc->cleanup();
            StackPool::pushStack(newProc->kernelStackBase);
            frame->rax = (uint64_t)(-1);
            return;
        }
        uint64_t stackOffset = currentProc->kernelStackTop - frame->rsp;
        uint64_t newStackLocation = newProc->kernelStackTop - stackOffset;
        newProc->state.extendedRegs.loadFromFPU();
        memcpy((void *)newStackLocation, (void *)frame->rsp, stackOffset);
        newProc->state.generalRegs.rsp = newStackLocation;
        frame->rax = newProcessID;
        ProcessManager::addToRunList(newProcessID);
    }
    extern "C" void YY_Yield() {
        core::log("YY_Yield();\n\r");
        proc::ProcessManager::yield();
    }
    extern "C" uint64_t YY_QueryAPIInfo(uint64_t id) {
        core::log("YY_QueryAPIInfo(%llu);\n\r", id);
        switch (id) {
        case YY_APIInfoId_PageSize:
            return 0x1000;
        case YY_APIInfoID_MaxArgCount:
            return YY_MaxArgCount;
        case YY_APIInfoId_MaxArgLength:
            return YY_MaxArgLength;
        case YY_APIInfoID_ExecMaxPathLength:
            return YY_ExecMaxPathLength;
        default:
            // API Info at this id is not supported
            return (uint64_t)(-1);
        }
    }

    extern "C" uint64_t YY_VirtualAlloc(uint64_t pagesCount, uint64_t flags) {
        core::log("YY_VirtualAlloc(%llu, %llu);\n\r", pagesCount, flags);
        Process *proc = proc::ProcessManager::getRunningProcess();
        memory::vaddr_t result = proc->usralloc->alloc(pagesCount * 0x1000);
        if (result == 0) {
            return 0;
        }
        uint64_t mask = (1ULL << 0) | (1ULL << 2);
        if ((flags & YY_VirtualFlagsWritable) != 0) {
            mask |= (1ULL << 1);
        }
        if ((flags & YY_VirtualFlagsExecutable) == 0) {
            mask |= (1ULL << 63);
        }
        if (!memory::VirtualMemoryMapper::mapPages(
                result, result + pagesCount * 0x1000, 0, mask)) {
            proc->usralloc->free(result, pagesCount * 0x1000);
            return 0;
        }
        return result;
    }
    extern "C" int64_t YY_VirtualFree(uint64_t start, uint64_t pagesCount) {
        core::log("YY_VirtualFree(%p, %llu)\n\r", start, pagesCount);
        if (pagesCount == 0) {
            return -1;
        }
        if (!memory::virtualRangeConditionCheck(start, pagesCount * 0x1000,
                                                false, false, false)) {
            return -1;
        }
        memory::VirtualMemoryMapper::freePages(start, pagesCount * 0x1000);
        Process *proc = proc::ProcessManager::getRunningProcess();
        if (!(proc->usralloc->free(start, pagesCount * 0x1000))) {
            // not enough memory to free the memory
            // that is unfortunate
            proc::ProcessManager::exit();
        }
        return 1;
    }
    extern "C" uint64_t YY_CheckProcStatus(uint64_t pid) {
        core::log("YY_CheckProcStatus(%llu)\n\r", pid);
        Process *proc = ProcessManager::getProcessData(pid);
        if (proc->ppid != ProcessManager::getRunningProcess()->pid) {
            return (uint64_t)(-1);
        }
        uint64_t dead = proc->dead;
        if (dead == 1) {
            ProcessManager::freePid(pid);
        }
        return dead;
    }
    extern "C" uint64_t YY_ExecuteBinary(const char *path, uint64_t argc,
                                         const char **argv) {
        core::log("YY_ExecuteBinary(%p, %llu, %p);\n\r", path, argc, argv);
        // TODO: check pages while calculating length
        if (strlen(path, YY_ExecMaxPathLength + 1) > YY_ExecMaxPathLength) {
            return (uint64_t)(-1);
        }
        if (!memory::virtualRangeConditionCheck(
                (memory::vaddr_t)argv, 8 * (argc + 1), true, false, false)) {
            return (uint64_t)(-1);
        }
        Process *proc = ProcessManager::getRunningProcess();
        fs::IFile *binary = nullptr;
        Elf *elf = nullptr;
        bool recovarable = true;

        if (argc > YY_MaxArgCount) {
            return (uint64_t)(-1);
        }
        for (uint64_t i = 0; i < argc; ++i) {
            // TODO: check pages while calculating length
            if (strlen(argv[i], YY_MaxArgLength + 1) > YY_MaxArgLength) {
                return (uint64_t)(-1);
            }
        }

        char **argsTmp = new char *[argc + 1];
        memory::vaddr_t argsStorage = 0;
        char **argsPointers = nullptr;
        char *argsValues = nullptr;

        memset(argsTmp, sizeof(char *) * (argc + 1), '\0');
        uint64_t fullsizeof = 8 * (argc + 1);
        for (uint64_t i = 0; i < argc; ++i) {
            fullsizeof += (strlen(argv[i]) + 1);
            argsTmp[i] = strdup(argv[i]);
            if (argsTmp[i] == 0) {
                goto failureArgsCleanup;
            }
        }
        fullsizeof = alignUp(fullsizeof, 0x1000);
        argsTmp[argc] = NULL;

        binary = fs::VFS::open(path, 1);
        if (binary == nullptr) {
            goto failureArgsCleanup;
        }
        memory::CoW::deallocateUserMemory();
        recovarable = false;
        delete proc->usralloc;
        proc->usralloc = memory::newUserVirtualAllocator();
        if (proc->usralloc == nullptr) {
            goto failureFileCleanup;
        }
        elf = parseElf(binary);
        if (elf == nullptr) {
            goto failureUsrallocCleanup;
        }
        if (!elf->load(binary, proc->usralloc)) {
            goto failureElfCleanup;
        }
        argsStorage = proc->usralloc->alloc(fullsizeof);
        if (argsStorage == 0) {
            goto failureElfCleanup;
        }
        if (!memory::VirtualMemoryMapper::mapPages(
                argsStorage, argsStorage + fullsizeof, 0, (0x7))) {
            goto failureElfCleanup;
        }
        argsPointers = (char **)argsStorage;
        argsValues = (char *)(argsStorage + (sizeof(8) * (argc + 1)));
        for (uint64_t i = 0; i < argc; ++i) {
            argsPointers[i] = argsValues;
            uint64_t spaceUsed = strlen(argsTmp[i]) + 1;
            memcpy(argsPointers[i], argsTmp[i], spaceUsed);
            argsValues += spaceUsed;
        }
        delete binary;
        delete elf;
        for (uint64_t i = 0; i < argc; ++i) {
            if (argsTmp[i] != 0) {
                delete argsTmp[i];
            }
        }
        delete argsTmp;
        jumpToUserMode(elf->head.entryPoint, argc, argsPointers);
    failureElfCleanup:
        delete elf;
    failureUsrallocCleanup:
        delete proc->usralloc;
    failureFileCleanup:
        delete binary;
    failureArgsCleanup:
        for (uint64_t i = 0; i < argc; ++i) {
            if (argsTmp[i] != 0) {
                delete argsTmp[i];
            }
        }
        delete argsTmp;
        if (recovarable) {
            return (uint64_t)(-1);
        } else {
            proc::ProcessManager::exit();
        }
    }
}; // namespace proc