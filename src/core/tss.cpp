#include <core/tss.hpp>

namespace core {
    TSSLayout TSS::tss;
    uint64_t TSS::getBase() { return (uint64_t)(&tss); }
    extern "C" void loadTSS();
    void TSS::init() {
        memset(&tss, sizeof(tss), 0);
        loadTSS();
    }
    void TSS::setKernelStack(uint64_t rsp) { tss.rsp[0] = rsp; }
}; // namespace core