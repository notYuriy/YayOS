#include <x86_64/tss.hpp>

namespace x86_64 {
    TSSLayout TSS::tss;
    uint64_t TSS::getBase() { return (uint64_t)(&tss); }
    extern "C" void loadTSS();
    void TSS::init() {
        memset(&tss, sizeof(tss), 0);
        tss.iopb = sizeof(tss);
        loadTSS();
    }
    void TSS::setKernelStack(uint64_t rsp) { tss.rsp[0] = rsp; }
}; // namespace x86_64