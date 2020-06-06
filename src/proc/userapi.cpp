#include <memory/cow.hpp>
#include <memory/kvmmngr.hpp>
#include <memory/msecurity.hpp>
#include <proc/proc.hpp>
#include <proc/stackpool.hpp>
#include <proc/userapi.hpp>

namespace proc {
    constexpr uint64_t ENOMEM = 12;
    constexpr uint64_t EAGAIN = 38;

    [[noreturn]] void YY_ExitProcess() { proc::ProcessManager::exit(); }
    extern "C" int64_t YY_ConsoleWrite(char *location, uint64_t size) {
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
            return -1;
        }
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
        pid_t newProcessID = ProcessManager::newProcess();
        if (newProcessID == pidCount) {
            frame->rax = (uint64_t)(-EAGAIN);
            return;
        }
        Process *newProc = ProcessManager::getProcessData(newProcessID);
        Process *currentProc = ProcessManager::getRunningProcess();
        if (!newProc->setup(false)) {
            ProcessManager::freePid(newProcessID);
            frame->rax = (uint64_t)(-ENOMEM);
            return;
        }

        if ((newProc->usralloc = currentProc->usralloc->copy()) == nullptr) {
            newProc->cleanup();
            StackPool::pushStack(newProc->kernelStackBase);
            frame->rax = (uint64_t)(-ENOMEM);
            return;
        }
        newProc->pid = newProcessID;
        frame->rax = 0;
        newProc->state.generalRegs.copyFrom(frame);
        newProc->state.generalRegs.cr3 = memory::CoW::clonePageTable();
        if (!newProc->state.generalRegs.cr3) {
            while (1)
                ;
            newProc->cleanup();
            StackPool::pushStack(newProc->kernelStackBase);
            frame->rax = (uint64_t)(-ENOMEM);
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
    extern "C" void YY_Yield() { proc::ProcessManager::yield(); }
}; // namespace proc