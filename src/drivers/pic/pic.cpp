#include <pic.hpp>
#include <pic8259.hpp>
#include <kheap.hpp>

namespace drivers {
    PIC* PIC::systemPic;
    bool PIC::picInitialized;

    void PIC::detectPIC() {
        if(!interrupts::IDT::isInitialized()) {
            panic("[PIC] Dependency \"IDT\" is not satisfied");
        }
        if(!memory::KernelHeap::isInitialized()) {
            panic("[PIC] Dependency \"KernelHeap\" is not satisfied");
        }
        PIC8259* pic = new PIC8259;
        pic->init();
        systemPic = pic;
        if(!systemPic->isInstanceInitialized()) {
            panic("[PIC] Failed to initialize 8259 controller\n\r");
        }
        picInitialized = true;
    }
}