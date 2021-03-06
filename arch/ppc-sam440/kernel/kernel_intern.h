#ifndef KERNEL_INTERN_H_
#define KERNEL_INTERN_H_

#include <aros/libcall.h>
#include <inttypes.h>
#include <exec/lists.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <utility/tagitem.h>
#include <asm/amcc440.h>
#include <stdio.h>
#include <stdarg.h>

#include "syscall.h"

#define KERNEL_PHYS_BASE        0x00800000
#define KERNEL_VIRT_BASE        0xff800000

#define STACK_SIZE 4096

struct KernelBase {
    struct Node         kb_Node;
    void *              kb_MemPool;
    struct List         kb_Exceptions[16];
    struct List         kb_Interrupts[64];
    struct MemHeader    *kb_SupervisorMem;
};

struct KernelBSS {
    void *addr;
    uint32_t len;
};

enum intr_types {
    it_exception = 0xe0,
    it_interrupt = 0xf0
};

struct IntrNode {
    struct MinNode      in_Node;
    void                (*in_Handler)(void *, void *);
    void                *in_HandlerData;
    void                *in_HandlerData2;
    uint8_t             in_type;
    uint8_t             in_nr;
};

static inline struct KernelBase *getKernelBase()
{
    return (struct KernelBase *)rdspr(SPRG4U);
}

static inline struct KernelBase *getSysBase()
{
    return (struct KernelBase *)rdspr(SPRG5U);
}

static inline void goSuper() {
    asm volatile("li %%r3,%0; sc"::"i"(SC_SUPERSTATE):"memory","r3");
}

static inline void goUser() {
    wrmsr(rdmsr() | (MSR_PR));
}


intptr_t krnGetTagData(Tag tagValue, intptr_t defaultVal, const struct TagItem *tagList);
struct TagItem *krnFindTagItem(Tag tagValue, const struct TagItem *tagList);
struct TagItem *krnNextTagItem(const struct TagItem **tagListPtr);

void core_LeaveInterrupt(regs_t *regs) __attribute__((noreturn));
void core_Switch(regs_t *regs) __attribute__((noreturn));
void core_Schedule(regs_t *regs) __attribute__((noreturn));
void core_Dispatch(regs_t *regs) __attribute__((noreturn));
void core_ExitInterrupt(regs_t *regs) __attribute__((noreturn)); 
void core_Cause(struct ExecBase *SysBase);
void mmu_init(struct TagItem *tags);
void intr_init();

void __attribute__((noreturn)) syscall_handler(regs_t *ctx, uint8_t exception, void *self);
void __attribute__((noreturn)) uic_handler(regs_t *ctx, uint8_t exception, void *self);

#ifdef bug
#undef bug
#endif
#ifdef D
#undef D
#endif
#define D(x) x

AROS_LD2(int, KrnBug,
         AROS_LDA(const char *, format, A0),
         AROS_LDA(va_list, args, A1),
         struct KernelBase *, KernelBase, 11, Kernel);

static inline void bug(const char *format, ...)
{
    struct KernelBase *kbase = getKernelBase();
    va_list args;
    va_start(args, format);
    AROS_SLIB_ENTRY(KrnBug, Kernel)(format, args, kbase);
    va_end(args);
}

#endif /*KERNEL_INTERN_H_*/
