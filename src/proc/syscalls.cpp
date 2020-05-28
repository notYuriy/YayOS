#include <memory/cow.hpp>
#include <memory/kvmmngr.hpp>
#include <proc/proc.hpp>
#include <proc/syscalls.hpp>

namespace proc {
    constexpr uint64_t ENOMEM = 12;
    constexpr uint64_t EAGAIN = 38;

    [[noreturn]] void sysExit() { proc::ProcessManager::exit(); }
    int64_t sysHelloWorld() {
        core::log("Hello world!\n\r");
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
            memory::KernelVirtualAllocator::unmapAt(newProc->kernelStackBase,
                                                    newProc->kernelStackSize);
            frame->rax = (uint64_t)(-ENOMEM);
            return;
        }
        newProc->pid = newProcessID;
        frame->rax = 0;
        newProc->state.generalRegs.copyFrom(frame);
        newProc->state.generalRegs.cr3 = memory::CoW::clonePageTable();
        uint64_t stackOffset = currentProc->kernelStackTop - frame->rsp;
        uint64_t newStackLocation = newProc->kernelStackTop - stackOffset;
        newProc->state.extendedRegs.loadFromFPU();
        memcpy((void *)newStackLocation, (void *)frame->rsp, stackOffset);
        newProc->state.generalRegs.rsp = newStackLocation;
        frame->rax = newProcessID;
        ProcessManager::addToRunList(newProcessID);
    }
}; // namespace proc