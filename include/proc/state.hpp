#ifndef __REGS_HPP_INCLUDED__
#define __REGS_HPP_INCLUDED__

#include <utils.hpp>

namespace proc {

#pragma pack(1)

    struct GeneralRegs {
        uint64_t fs, gs, ds, es;
        uint64_t cr3;
        uint64_t r15, r14;
        uint64_t r13, r12, r11, r10, r9, r8;
        uint64_t rbp, rsi, rdi, rdx, rcx, rbx, rax;
        uint64_t rip, cs, rflags, rsp, ss;
        INLINE void zero() { memset(this, sizeof(GeneralRegs), 0); }
        INLINE void copyFrom(GeneralRegs *regs) {
            memcpy(this, regs, sizeof(*this));
        }
    };

    typedef GeneralRegs SchedulerIntFrame;

    extern "C" void extendedRegsLoadFromFpu(char *loc);
    extern "C" void extendedRegsSaveToFpu(char *loc);

    struct ExtendedRegs {
        char buf[512];
        INLINE void loadFromFPU() { extendedRegsLoadFromFpu(buf); }
        INLINE void loadToFPU() { extendedRegsSaveToFpu(buf); }
        INLINE void zero() { memset(this, sizeof(ExtendedRegs), 0); }
    };

    struct TaskState {
        ExtendedRegs extendedRegs;
        GeneralRegs generalRegs;

        INLINE void loadToFrame(SchedulerIntFrame *frame) {
            memcpy(frame, &generalRegs, sizeof(*frame));
            extendedRegs.loadToFPU();
        }

        INLINE void loadFromFrame(SchedulerIntFrame *frame) {
            memcpy(&generalRegs, frame, sizeof(*frame));
            extendedRegs.loadFromFPU();
        }

        INLINE void copyFrom(TaskState *other) {
            generalRegs.copyFrom(&(other->generalRegs));
            extendedRegs.loadFromFPU();
        }
    };

#pragma pack(0)

}; // namespace proc

#endif