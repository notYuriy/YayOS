#include <core/interrupts.hpp>
#include <core/log.hpp>
#include <drivers/pic/pic.hpp>
#include <drivers/pic/pic8259.hpp>
#include <drivers/serial.hpp>
#include <drivers/timer/pit.hpp>
#include <mm/kvmmngr.hpp>
#include <mm/mminit.hpp>
#include <proc/mutex.hpp>
#include <proc/sched.hpp>

proc::Mutex mutex;
bool finished;

void thread1() {
    for (uint64_t i = 0; i < 1000; ++i) {
        mutex.lock();
        core::log("Thread 2: %llu\n\r", i);
        mutex.unlock();
    }
    finished = true;
    while(1) {}
}

extern "C" void kmain(uint64_t mbPointer) {
    drivers::Serial::init(drivers::SerialPort::COM1);
    memory::init(mbPointer);
    finished = false;
    core::IDT::init();
    drivers::IPIC::detectPIC();
    drivers::PIT timer;
    timer.init(200);
    proc::Scheduler::init(&timer);
    timer.enable();
    proc::Task *newTask = new proc::Task;
    newTask->state.generalRegs.cr3 = getPageTable();
    newTask->state.generalRegs.cs = getCS();
    newTask->state.generalRegs.ds = getDS();
    newTask->state.generalRegs.es = getES();
    newTask->state.generalRegs.fs = getFS();
    newTask->state.generalRegs.gs = getGS();
    newTask->state.generalRegs.ss = getSS();
    newTask->state.generalRegs.rflags = getFlags();
    newTask->state.generalRegs.rip = (uint64_t)thread1;
    newTask->state.generalRegs.rsp = memory::KernelVirtualAllocator::getMapping(
                                         8192, 0, memory::defaultKernelFlags) +
                                     4096;
    mutex.init();
    uint64_t time1 = timer.getTimeInMilliseconds();
    proc::Scheduler::addToRunList(newTask);
    for (uint64_t i = 0; i < 1000; ++i) {
        mutex.lock();
        core::log("Thread 1: %llu\n\r", i);
        mutex.unlock();
    }
    while (!finished) {
        core::log("Finished\n\r");
    }
    uint64_t time2 = timer.getTimeInMilliseconds();
    core::log("Time: %llu", time2 - time1);
    while (true) {}

}
