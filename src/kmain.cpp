#include <interrupts.hpp>
#include <kprintf.hpp>
#include <mminit.hpp>
#include <pic.hpp>
#include <pic8259.hpp>
#include <pit.hpp>
#include <proc.hpp>
#include <serial.hpp>

void kekich() {
    while(1) {
        kprintf("Thread 2\n\r");
        ///proc::ProcessManager::yield();
    }
}

extern "C" void kmain(Uint64 mbPointer) {
    IO::Serial::init(IO::SerialPort::COM1);
    memory::init(mbPointer);
    interrupts::IDT::init();
    drivers::PIC::detectPIC();
    drivers::PIT timer;
    timer.init(20);
    proc::ProcessManager::init(&timer);
    timer.enable();

    proc::Process* newProc = proc::ProcessManager::newProc();
    proc::Pid pid = newProc->pid;
    kprintf("pid: %ull\n\r", pid);

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
    kprintf("%ull %ull %ull %ull %ull %ull\n\r", getCS(), getDS(), getFS(), getGS(), getES(), getSS());
    newProc->state.generalRegs.rip = (Uint64)kekich;
    proc::ProcessManager::addToRunList(newProc);
    while(1) {
        kprintf("Thread 1\n\r");
        //proc::ProcessManager::yield();
    }
}