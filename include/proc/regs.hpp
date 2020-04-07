#ifndef __REGS_HPP_INCLUDED__
#define __REGS_HPP_INCLUDED__

#include <utils.hpp>

namespace proc {

#pragma pack(1)

    struct Regs {
        Uint64 fs, gs, ds, es;
        Uint64 intno;
        Uint64 cr4, cr3, cr2, cr0;
        Uint64 r15, r14;
        Uint64 r13, r12, r11, r10, r9, r8;
        Uint64 rbp, rsi, rdi, rdx, rcx, rbx, rax;
        Uint64 errcode;
        Uint64 rip, cs, rflags, rsp, ss;
    };

    extern "C" void extendedRegsLoadFromFpu(char* loc);
    extern "C" void extendedRegsSaveToFpu(char* loc);

    struct ExtendedRegs {
        char buf[512];

        INLINE void loadFromFPU() {
            extendedRegsLoadFromFpu(buf);
        }

        INLINE void saveToFPU() {
            extendedRegsSaveToFpu(buf);
        }

    };

#pragma pack(0)

};

#endif