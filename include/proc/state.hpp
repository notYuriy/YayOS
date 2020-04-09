#ifndef __REGS_HPP_INCLUDED__
#define __REGS_HPP_INCLUDED__

#include <utils.hpp>

namespace proc {

#pragma pack(1)

    struct GeneralRegs {
        Uint64 fs, gs, ds, es;
        Uint64 cr3;
        Uint64 r15, r14;
        Uint64 r13, r12, r11, r10, r9, r8;
        Uint64 rbp, rsi, rdi, rdx, rcx, rbx, rax;
        Uint64 rip, cs, rflags, rsp, ss;
    };

    typedef GeneralRegs SchedulerIntFrame;

    extern "C" void extendedRegsLoadFromFpu(char* loc);
    extern "C" void extendedRegsSaveToFpu(char* loc);

    struct ExtendedRegs {
        char buf[512];
        INLINE void loadFromFPU() { extendedRegsLoadFromFpu(buf); }
        INLINE void loadToFPU() { extendedRegsSaveToFpu(buf); }
    };

    struct ProcessState {
        ExtendedRegs extendedRegs;
        GeneralRegs generalRegs;

        INLINE void loadToFrame(SchedulerIntFrame* frame) {
            memcpy(frame, &generalRegs, sizeof(*frame));
            extendedRegs.loadToFPU();
        }

        INLINE void loadFromFrame(SchedulerIntFrame* frame) {
            memcpy(&generalRegs, frame, sizeof(*frame));
            extendedRegs.loadFromFPU();
        }
    };

#pragma pack(0)

}; // namespace proc

#endif