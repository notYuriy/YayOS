#include <interrupts.hpp>
#include <log.hpp>
#include <mminit.hpp>
#include <pic.hpp>
#include <pic8259.hpp>
#include <pit.hpp>
#include <proc.hpp>
#include <serial.hpp>

void kekich() {
    while(1) {
        core::log("2");
        ///proc::ProcessManager::yield();
    }
}

extern "C" void kmain(Uint64 mbPointer) {
    drivers::Serial::init(drivers::SerialPort::COM1);
    memory::init(mbPointer);
    core::IDT::init();
    drivers::PIC::detectPIC();
    drivers::PIT timer;
    timer.init(20);
    proc::ProcessManager::init(&timer);
    timer.enable();

    proc::Process* newProc = proc::ProcessManager::newProc();
    proc::Pid pid = newProc->pid;
    core::log("pid: %ull\n\r", pid);

    memset(newProc->state.extendedRegs.buf,
           sizeof(newProc->state.extendedRegs.buf), 0);

    memset(&(newProc->state.generalRegs), sizeof(newProc->state.generalRegs),
           0);

    newProc->state.generalRegs.rsp = memory::KernelVirtualAllocator::getMapping(
        4096, 0, memory::defaultKernelFlags) + 4096;

    newProc->state.generalRegs.cr3 = getPageTable();
    newProc->state.generalRegs.rflags = getFlags();
    newProc->state.generalRegs.cs = getCS();
    newProc->state.generalRegs.ds = getDS();
    newProc->state.generalRegs.fs = getFS();
    newProc->state.generalRegs.gs = getGS();
    newProc->state.generalRegs.es = getES();
    newProc->state.generalRegs.ss = getSS();
    newProc->state.generalRegs.rip = (Uint64)kekich;

    proc::ProcessManager::addToRunList(newProc);
    for(Uint64 i = 0; i < 100000; ++i) {
        core::log("1");
    }
    //proc::ProcessManager::suspendFromRunList(pid);
    while(1) {}
}