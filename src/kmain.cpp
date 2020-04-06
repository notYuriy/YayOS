#include <interrupts.hpp>
#include <kprintf.hpp>
#include <madt.hpp>
#include <mminit.hpp>
#include <pic.hpp>
#include <pictimer.hpp>
#include <roottable.hpp>
#include <rsdp.hpp>
#include <serial.hpp>
#include <lapic.hpp>

extern "C" void kmain(Uint64 mbPointer) {
    IO::Serial::init(IO::SerialPort::COM1);
    memory::init(mbPointer);
    interrupts::IDT::init();
    drivers::PIC::detectPIC();
    drivers::PICTimer timer;
    timer.init(20);
    acpi::RSDPManager::init(
        (multiboot::BootInfoHeader*)(mbPointer + KERNEL_MAPPING_BASE));
    acpi::RootTableManager::init();
    acpi::MADTManager::init();
    drivers::LAPIC::init();
    if(drivers::LAPIC::isInitialized()) {
        kprintf("initialized LAPIC\n\r");
    }
    kprintf("It didn't crash\n\r");
    kprintf("LAPIC addr: %p\n\r", acpi::MADTManager::getLAPICAddr());
    while (1) {}
}