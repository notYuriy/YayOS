#include <drivers/pic/pic.hpp>
#include <drivers/pic/pic8259.hpp>
#include <mm/kheap.hpp>

namespace drivers {
    IPIC *IPIC::systemPic;
    bool IPIC::picInitialized;

    void IPIC::detectPIC() {
        if (!core::IDT::isInitialized()) {
            panic("[PIC] Dependency \"IDT\" is not satisfied");
        }
        if (!memory::KernelHeap::isInitialized()) {
            panic("[PIC] Dependency \"KernelHeap\" is not satisfied");
        }
        PIC8259 *pic = new PIC8259;
        pic->init();
        systemPic = pic;
        if (!systemPic->isInstanceInitialized()) {
            panic("[PIC] Failed to initialize 8259 controller\n\r");
        }
        picInitialized = true;
    }
} // namespace drivers