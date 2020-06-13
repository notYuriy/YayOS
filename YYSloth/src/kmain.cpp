#include <core/log.hpp>
#include <drivers/pic/pic.hpp>
#include <drivers/pic/pic8259.hpp>
#include <drivers/serial.hpp>
#include <drivers/timer/pit.hpp>
#include <fs/ramdiskfs.hpp>
#include <fs/vfs.hpp>
#include <memory/cow.hpp>
#include <memory/kvmmngr.hpp>
#include <memory/mminit.hpp>
#include <memory/usrvmmngr.hpp>
#include <proc/elf.hpp>
#include <proc/mutex.hpp>
#include <proc/proc.hpp>
#include <proc/stackpool.hpp>
#include <proc/usermode.hpp>
#include <x86_64/gdt.hpp>
#include <x86_64/interrupts.hpp>
#include <x86_64/syscall.hpp>
#include <x86_64/tss.hpp>

void initProcess() {
    fs::IFile *file = fs::VFS::open("Y:\\Binaries\\init", 0);
    if (file == nullptr) {
        panic(
            "[KernelInit] Failed to load init process executable from ramdisk");
    }
    proc::Elf *elf = proc::parseElf(file);
    if (elf == nullptr) {
        panic("[KernelInit] Failed to parse init process executable");
    }
    if (!elf->load(file, proc::ProcessManager::getRunningProcess()->usralloc)) {
        panic("[KernelInit] Failed to load init process executable to memory");
    }

    char **fakeArgs =
        (char **)proc::ProcessManager::getRunningProcess()->usralloc->alloc(
            0x1000);
    memory::VirtualMemoryMapper::mapPages((memory::vaddr_t)fakeArgs,
                                          ((memory::vaddr_t)fakeArgs) + 0x1000,
                                          0, (0x7));
    fakeArgs[0] = NULL;
    proc::jumpToUserMode(elf->head.entryPoint, 0, fakeArgs);
}

extern "C" void kmain(uint64_t mbPointer, void (**ctorsStart)(),
                      void (**ctorsEnd)()) {
    executeCtors(ctorsStart, ctorsEnd);
    drivers::Serial::init(drivers::SerialPort::COM1);
    // drivers::Serial::recieve(drivers::SerialPort::COM1);
    // drivers::Serial::recieve(drivers::SerialPort::COM1);
    memory::init(mbPointer);
    x86_64::GDT::init();
    x86_64::TSS::init();
    x86_64::IDT::init();
    memory::CoW::init();
    x86_64::SyscallTable::init();
    drivers::IPIC::detectPIC();
    drivers::IPIC::getSystemPIC()->enableLegacyIrq(8);
    drivers::PIT timer;
    timer.init(200);
    proc::ProcessManager::init(&timer);
    proc::StackPool::init();
    fs::RamdiskFsSuperblock initRd;
    fs::VFS::init();
    fs::VFS::mount('Y', &initRd);
    proc::pid_t initProcessPid = proc::ProcessManager::newProcess();
    proc::Process *initProcessData =
        proc::ProcessManager::getProcessData(initProcessPid);
    initProcessData->setup();
    initProcessData->state.generalRegs.zero();
    initProcessData->state.extendedRegs.zero();
    initProcessData->state.generalRegs.cs = getCS();
    initProcessData->state.generalRegs.ds = getDS();
    initProcessData->state.generalRegs.es = getES();
    initProcessData->state.generalRegs.fs = getFS();
    initProcessData->state.generalRegs.ss = getSS();
    initProcessData->state.generalRegs.gs = getGS();
    initProcessData->state.generalRegs.cr3 = memory::CoW::newPageTable();
    initProcessData->state.generalRegs.rip = (uint64_t)initProcess;
    initProcessData->state.generalRegs.rflags = getFlags();
    initProcessData->pid = initProcessPid;
    initProcessData->ppid = 0;
    initProcessData->dead = 0;

    // this stack will only be used to setup the process
    initProcessData->state.generalRegs.rsp = initProcessData->kernelStackTop;
    timer.enable();
    proc::ProcessManager::addToRunList(initProcessPid);
    proc::ProcessManager::yield();
    // at this point this task is only executed
    // if there is no other task to run
    // or there are free stacks in stackpool
    while (true) {
        proc::ProcessManager::yield();
        while (proc::StackPool::freeStack()) {
            asm("pause" :::);
        }
        asm("pause" :::);
    }
}
