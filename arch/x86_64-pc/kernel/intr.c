#include <inttypes.h>
#define DEBUG 0
#include <aros/debug.h>

#include <stdio.h>
#include <asm/cpu.h>
#include <asm/io.h>
#include <asm/segments.h>
#include <aros/libcall.h>
#include <aros/asmcall.h>
#include <exec/execbase.h>
#include <hardware/intbits.h>

#include <proto/exec.h>

#include "kernel_intern.h"

static void core_XTPIC_DisableIRQ(uint8_t irqnum);
static void core_XTPIC_EnableIRQ(uint8_t irqnum);

AROS_LH4(void *, KrnAddIRQHandler,
         AROS_LHA(uint8_t, irq, D0),
         AROS_LHA(void *, handler, A0),
         AROS_LHA(void *, handlerData, A1),
         AROS_LHA(void *, handlerData2, A2),
         struct KernelBase *, KernelBase, 7, Kernel)
{
    AROS_LIBFUNC_INIT
    
    struct ExecBase *SysBase = TLS_GET(SysBase);
    struct IntrNode *handle = NULL;
    D(bug("[Kernel] KrnAddIRQHandler(%02x, %012p, %012p, %012p):\n", irq, handler, handlerData, handlerData2));
    
    if (irq >=0 && irq <= 0xff)
    {
    
        handle = AllocVecPooled(KernelBase->kb_MemPool, sizeof(struct IntrNode));
        
#warning TODO: Add IP range checking
        
        D(bug("[Kernel]   handle=%012p\n", handle));
        
        if (handle)
        {
            handle->in_Handler = handler;
            handle->in_HandlerData = handlerData;
            handle->in_HandlerData2 = handlerData2;
            
            Disable();
            ADDHEAD(&KernelBase->kb_Intr[irq], &handle->in_Node);
            Enable();
            
            if (KernelBase->kb_Intr[irq].lh_Type == KBL_XTPIC)
                core_XTPIC_EnableIRQ(irq);
        }
    }
    return handle;
    
    AROS_LIBFUNC_EXIT
}

AROS_LH1(void, KrnRemIRQHandler,
         AROS_LHA(void *, handle, A0),
         struct KernelBase *, KernelBase, 8, Kernel)
{
    AROS_LIBFUNC_INIT

    struct IntrNode *h = handle;
    
    Disable();
    REMOVE(h);
    Enable();

    FreeVecPooled(KernelBase->kb_MemPool, h);
    
    AROS_LIBFUNC_EXIT
}

AROS_LH0I(void, KrnCli,
         struct KernelBase *, KernelBase, 9, Kernel)
{
    AROS_LIBFUNC_INIT

    asm volatile("cli");
    
    AROS_LIBFUNC_EXIT
}

AROS_LH0I(void, KrnSti,
         struct KernelBase *, KernelBase, 10, Kernel)
{
    AROS_LIBFUNC_INIT

    asm volatile("sti");
    
    AROS_LIBFUNC_EXIT
}


static struct int_gate_64bit IGATES[256] __attribute__((used,aligned(256)));
const struct
{
    uint16_t size __attribute__((packed));
    uint64_t base __attribute__((packed));
} IDT_sel = {sizeof(IGATES)-1, (uint64_t)IGATES};

#define STR_(x) #x
#define STR(x) STR_(x)

#define IRQ_NAME_(nr) nr##_intr(void)
#define IRQ_NAME(nr) IRQ_NAME_(IRQ##nr)

#define BUILD_IRQ(nr) \
    void IRQ_NAME(nr); \
    asm(".balign 8  ,0x90\n\t" \
        ".globl IRQ" STR(nr) "_intr\n\t" \
        ".type IRQ" STR(nr) "_intr,@function\n" \
        "IRQ" STR(nr) "_intr: pushq $0; pushq $" #nr "\n\t" \
        "jmp core_EnterInterrupt\n\t" \
        ".size IRQ" STR(nr) "_intr, . - IRQ" STR(nr) "_intr" \
    );

#define BUILD_IRQ_ERR(nr) \
    void IRQ_NAME(nr); \
    asm(".balign 8  ,0x90\n\t" \
        ".globl IRQ" STR(nr) "_intr\n\t" \
        ".type IRQ" STR(nr) "_intr,@function\n" \
        "IRQ" STR(nr) "_intr: pushq $" #nr "\n\t" \
        "jmp core_EnterInterrupt\n\t" \
        ".size IRQ" STR(nr) "_intr, . - IRQ" STR(nr) "_intr" \
    );

BUILD_IRQ(0x00)         // Divide-By-Zero Exception
BUILD_IRQ(0x01)         // Debug Exception
BUILD_IRQ(0x02)         // NMI Exception
BUILD_IRQ(0x03)         // Breakpoint Exception
BUILD_IRQ(0x04)         // Overflow Exception
BUILD_IRQ(0x05)         // Bound-Range Exception
BUILD_IRQ(0x06)         // Invalid-Opcode Exception
BUILD_IRQ(0x07)         // Device-Not-Available Exception
BUILD_IRQ_ERR(0x08)     // Double-Fault Exception
BUILD_IRQ(0x09)         // Unused (used to be Coprocesor-Segment-Overrun)
BUILD_IRQ_ERR(0x0a)     // Invalid-TSS Exception
BUILD_IRQ_ERR(0x0b)     // Segment-Not-Present Exception
BUILD_IRQ_ERR(0x0c)     // Stack Exception
BUILD_IRQ_ERR(0x0d)     // General-Protection Exception
BUILD_IRQ_ERR(0x0e)     // Page-Fault Exception
BUILD_IRQ(0x0f)         // Reserved
BUILD_IRQ(0x10)         // Floating-Point Exception
BUILD_IRQ_ERR(0x11)     // Alignment-Check Exception
BUILD_IRQ(0x12)         // Machine-Check Exception
BUILD_IRQ(0x13)         // SIMD-Floating-Point Exception
BUILD_IRQ(0x14) BUILD_IRQ(0x15) BUILD_IRQ(0x16) BUILD_IRQ(0x17) 
BUILD_IRQ(0x18) BUILD_IRQ(0x19) BUILD_IRQ(0x1a) BUILD_IRQ(0x1b)
BUILD_IRQ(0x1c) BUILD_IRQ(0x1d) BUILD_IRQ(0x1e) BUILD_IRQ(0x1f)

#define B(x,y) BUILD_IRQ(x##y)
#define B16(x) \
    B(x,0) B(x,1) B(x,2) B(x,3) B(x,4) B(x,5) B(x,6) B(x,7) \
    B(x,8) B(x,9) B(x,a) B(x,b) B(x,c) B(x,d) B(x,e) B(x,f)

B16(0x2)
BUILD_IRQ(0x80)
BUILD_IRQ(0xfe) // APIC timer

#define SAVE_REGS        \
        "pushq %rax; pushq %rbp; pushq %rbx; pushq %rdi; pushq %rsi; pushq %rdx;"  \
        "pushq %rcx; pushq %r8; pushq %r9; pushq %r10; pushq %r11; pushq %r12;"  \
        "pushq %r13; pushq %r14; pushq %r15; mov %ds,%eax; pushq %rax;"
        
#define RESTORE_REGS    \
        "popq %rax; mov %ax,%ds; mov %ax,%es; popq %r15; popq %r14; popq %r13;"  \
        "popq %r12; popq %r11; popq %r10; popq %r9; popq %r8; popq %rcx;" \
        "popq %rdx; popq %rsi; popq %rdi; popq %rbx; popq %rbp; popq %rax"

asm(
"                .balign 32,0x90      \n"
"                .globl core_EnterInterrupt \n"
"                .type core_EnterInterrupt,@function \n"
"core_EnterInterrupt: \n\t" SAVE_REGS "\n"
"                movl    $" STR(KERNEL_DS) ",%eax \n"
"                mov     %ax,%ds \n"
"                mov     %ax,%es \n"
"                movq    %rsp,%rdi \n"
"                call    core_IRQHandle \n"
"                movq    %rsp,%rdi \n"
"                jmp     core_ExitInterrupt \n"
"                .size core_EnterInterrupt, .-core_EnterInterrupt"      
); 

asm(
"                .balign 32,0x90      \n"
"                .globl core_LeaveInterrupt \n"
"                .type core_LeaveInterrupt,@function \n"
"core_LeaveInterrupt: movq %rdi,%rsp \n\t" RESTORE_REGS "\n"                
"                addq $16,%rsp \n"
"                iretq \n"
"                .size core_LeaveInterrupt, .-core_LeaveInterrupt"
);

#define IRQ(x,y) \
    (const void (*)(void))IRQ##x##y##_intr

#define IRQLIST_16(x) \
    IRQ(x,0), IRQ(x,1), IRQ(x,2), IRQ(x,3), \
    IRQ(x,4), IRQ(x,5), IRQ(x,6), IRQ(x,7), \
    IRQ(x,8), IRQ(x,9), IRQ(x,a), IRQ(x,b), \
    IRQ(x,c), IRQ(x,d), IRQ(x,e), IRQ(x,f)

const void __attribute__((section(".text"))) (*interrupt[256])(void) = {
    IRQLIST_16(0x0), IRQLIST_16(0x1), IRQLIST_16(0x2)
};

void core_SetupIDT(struct KernBootPrivate *__KernBootPrivate)
{
    IPTR _APICBase;
    UBYTE _APICID;
    int i;
    uintptr_t off;

    _APICBase = AROS_UFC0(IPTR,
            ((struct GenericAPIC *)__KernBootPrivate->kbp_APIC_Drivers[__KernBootPrivate->kbp_APIC_DriverID])->getbase);

    _APICID = AROS_UFC1(UBYTE,
            ((struct GenericAPIC *)__KernBootPrivate->kbp_APIC_Drivers[__KernBootPrivate->kbp_APIC_DriverID])->getid,
                    AROS_UFCA(IPTR, _APICBase, A0));

    if (_APICID == __KernBootPrivate->kbp_APIC_BSPID)
    {
        rkprintf("[Kernel] core_SetupIDT[%d] Setting all interrupt handlers to default value\n", _APICID);

        for (i=0; i < 256; i++)
        {
            if (interrupt[i])
                off = (uintptr_t)interrupt[i];
            else if (i == 0x80)
                off = (uintptr_t)&IRQ0x80_intr;
            else if (i == 0xfe)
                off = (uintptr_t)&IRQ0xfe_intr;
            else
                off = (uintptr_t)&core_DefaultIRETQ;

            IGATES[i].offset_low = off & 0xffff;
            IGATES[i].offset_mid = (off >> 16) & 0xffff;
            IGATES[i].offset_high = (off >> 32) & 0xffffffff;
            IGATES[i].type = 0x0e;
            IGATES[i].dpl = 3;
            IGATES[i].p = 1;
            IGATES[i].selector = KERNEL_CS;
            IGATES[i].ist = 0;
        }
    }
    rkprintf("[Kernel] core_SetupIDT[%d] Registering interrupt handlers ..\n", _APICID);
    asm volatile ("lidt %0"::"m"(IDT_sel));
}


void core_Cause(struct ExecBase *SysBase)
{
    struct IntVector *iv = &SysBase->IntVects[INTB_SOFTINT];

    /* If the SoftInt vector in SysBase is set, call it. It will do the rest for us */
    if (iv->iv_Code)
    {
        AROS_UFC5(void, iv->iv_Code,
            AROS_UFCA(ULONG, 0, D1),
            AROS_UFCA(ULONG, 0, A0),
            AROS_UFCA(APTR, 0, A1),
            AROS_UFCA(APTR, iv->iv_Code, A5),
            AROS_UFCA(struct ExecBase *, SysBase, A6)
        );
    }
}

static void core_APIC_AckIntr(uint8_t intnum)
{
    struct KernelBase *KernelBase = TLS_GET(KernelBase);
    asm volatile ("movl %0,(%1)"::"r"(0),"r"(KernelBase->kb_APIC_BaseMap[0] + 0xb0));
}

static void core_XTPIC_DisableIRQ(uint8_t irqnum)
{
    struct KernelBase *KernelBase = TLS_GET(KernelBase);
    irqnum &= 15;
    KernelBase->kb_XTPIC_Mask |= 1 << irqnum;

    if (irqnum >= 8)
    {
        outb((KernelBase->kb_XTPIC_Mask >> 8) & 0xff, 0xA1);
    }
    else
    {
        outb(KernelBase->kb_XTPIC_Mask & 0xff, 0x21);
    }
}

static void core_XTPIC_EnableIRQ(uint8_t irqnum)
{
    struct KernelBase *KernelBase = TLS_GET(KernelBase);
    irqnum &= 15;
    KernelBase->kb_XTPIC_Mask &= ~(1 << irqnum);    

    if (irqnum >= 8)
    {
        outb((KernelBase->kb_XTPIC_Mask >> 8) & 0xff, 0xA1);
    }
    else
    {
        outb(KernelBase->kb_XTPIC_Mask & 0xff, 0x21);
    }
}

static void core_XTPIC_AckIntr(uint8_t intnum)
{
    struct KernelBase *KernelBase = TLS_GET(KernelBase);
    intnum &= 15;
    KernelBase->kb_XTPIC_Mask |= 1 << intnum;

    if (intnum >= 8)
    {
        outb((KernelBase->kb_XTPIC_Mask >> 8) & 0xff, 0xA1);
        outb(0x62, 0x20);
        outb(0x20, 0xa0);
    }
    else
    {
        outb(KernelBase->kb_XTPIC_Mask & 0xff, 0x21);
        outb(0x20, 0x20);
    }
}

void core_IRQHandle(regs_t regs)
{
    struct ExecBase *SysBase = TLS_GET(SysBase);        // *(struct ExecBase **)4;
    struct KernelBase *KernelBase = TLS_GET(KernelBase);
    struct Task *t = NULL;
    int die = 0;

    if (SysBase)
        t = SysBase->ThisTask;

    if ((regs.irq_number < 0x20) && regs.irq_number != 0x0e)
        rkprintf("IRQ %02x:", regs.irq_number);
    
    if (regs.irq_number == 0x03)        /* Debug */
    {
        rkprintf("[Kernel] INT3 debug fault!\n");

        if (t)
        {
            rkprintf("[Kernel]  %s %p '%s'\n",
                     t->tc_Node.ln_Type == NT_TASK?"task":"process", t, t->tc_Node.ln_Name);
            rkprintf("[Kernel]  SPLower=%016lx SPUpper=%016lx\n", t->tc_SPLower, t->tc_SPUpper);
            
            if (((uint64_t *)regs.return_rsp < t->tc_SPLower)||((uint64_t *)regs.return_rsp > t->tc_SPUpper))
                rkprintf("[Kernel] Stack out of Bounds!\n");
        }

        rkprintf("[Kernel]  stack=%04x:%012x rflags=%016x ip=%04x:%012x err=%08x\n",
                 regs.return_ss, regs.return_rsp, regs.return_rflags, 
                 regs.return_cs, regs.return_rip, regs.error_code);

        rkprintf("[Kernel]  rax=%016lx rbx=%016lx rcx=%016lx rdx=%016lx\n", regs.rax, regs.rbx, regs.rcx, regs.rdx);
        rkprintf("[Kernel]  rsi=%016lx rdi=%016lx rbp=%016lx rsp=%016lx\n", regs.rsi, regs.rdi, regs.rbp, regs.return_rsp);
        rkprintf("[Kernel]  r08=%016lx r09=%016lx r10=%016lx r11=%016lx\n", regs.r8, regs.r9, regs.r10, regs.r11);
        rkprintf("[Kernel]  r12=%016lx r13=%016lx r14=%016lx r15=%016lx\n", regs.r12, regs.r13, regs.r14, regs.r15);
        rkprintf("[Kernel]  *rsp=%016lx\n", *(uint64_t *)regs.return_rsp);


        rkprintf("[Kenrel] Stack:\n");
        int i;
        uint64_t *ptr = (void*)regs.return_rsp;
        for (i=0; i < 10; i++)
        {
            rkprintf("[Kernel]   %02x: %016p\n", i*8, ptr[i]);
        }
    }
    else if (regs.irq_number == 0x06)        /* GPF */
    {
        rkprintf("[Kernel] UNDEFINED INSTRUCTION!\n");
        
        if (t)
        {
            rkprintf("[Kernel]  %s %p '%s'\n",
                     t->tc_Node.ln_Type == NT_TASK?"task":"process", t, t->tc_Node.ln_Name);
            rkprintf("[Kernel]  SPLower=%016lx SPUpper=%016lx\n", t->tc_SPLower, t->tc_SPUpper);
            
            if (((uint64_t *)regs.return_rsp < t->tc_SPLower)||((uint64_t *)regs.return_rsp > t->tc_SPUpper))
                rkprintf("[Kernel] Stack out of Bounds!\n");
        }
        
        rkprintf("[Kernel]  stack=%04x:%012x rflags=%016x ip=%04x:%012x err=%08x\n",
                 regs.return_ss, regs.return_rsp, regs.return_rflags, 
                 regs.return_cs, regs.return_rip, regs.error_code);

        rkprintf("[Kernel]  rax=%016lx rbx=%016lx rcx=%016lx rdx=%016lx\n", regs.rax, regs.rbx, regs.rcx, regs.rdx);
        rkprintf("[Kernel]  rsi=%016lx rdi=%016lx rbp=%016lx rsp=%016lx\n", regs.rsi, regs.rdi, regs.rbp, regs.return_rsp);
        rkprintf("[Kernel]  r08=%016lx r09=%016lx r10=%016lx r11=%016lx\n", regs.r8, regs.r9, regs.r10, regs.r11);
        rkprintf("[Kernel]  r12=%016lx r13=%016lx r14=%016lx r15=%016lx\n", regs.r12, regs.r13, regs.r14, regs.r15);
        die = 1;
        
        int i;
        uint8_t *ptr = (uint8_t *)regs.return_rip;
        
        rkprintf("[Kernel]  ");
        
        for (i=0; i < 16; i++)
            rkprintf("%02x ", ptr[i]);
        
        rkprintf("\n");
    }
    else if (regs.irq_number == 0x07)        /* GPF */
    {
        rkprintf("[Kernel] Device Not Available!\n");
        
        if (t)
        {
            rkprintf("[Kernel]  %s %p '%s'\n",
                     t->tc_Node.ln_Type == NT_TASK?"task":"process", t, t->tc_Node.ln_Name);
            rkprintf("[Kernel]  SPLower=%016lx SPUpper=%016lx\n", t->tc_SPLower, t->tc_SPUpper);
            
            if (((uint64_t *)regs.return_rsp < t->tc_SPLower)||((uint64_t *)regs.return_rsp > t->tc_SPUpper))
                rkprintf("[Kernel] Stack out of Bounds!\n");
        }
        
        rkprintf("[Kernel]  stack=%04x:%012x rflags=%016x ip=%04x:%012x err=%08x\n",
                 regs.return_ss, regs.return_rsp, regs.return_rflags, 
                 regs.return_cs, regs.return_rip, regs.error_code);

        rkprintf("[Kernel]  rax=%016lx rbx=%016lx rcx=%016lx rdx=%016lx\n", regs.rax, regs.rbx, regs.rcx, regs.rdx);
        rkprintf("[Kernel]  rsi=%016lx rdi=%016lx rbp=%016lx rsp=%016lx\n", regs.rsi, regs.rdi, regs.rbp, regs.return_rsp);
        rkprintf("[Kernel]  r08=%016lx r09=%016lx r10=%016lx r11=%016lx\n", regs.r8, regs.r9, regs.r10, regs.r11);
        rkprintf("[Kernel]  r12=%016lx r13=%016lx r14=%016lx r15=%016lx\n", regs.r12, regs.r13, regs.r14, regs.r15);
        die = 1;
    }
    else if (regs.irq_number == 0x0D)        /* GPF */
    {
        rkprintf("[Kernel] GENERAL PROTECTION FAULT!\n");
        
        if (t)
        {
            rkprintf("[Kernel]  %s %p '%s'\n",
                     t->tc_Node.ln_Type == NT_TASK?"task":"process", t, t->tc_Node.ln_Name);
            rkprintf("[Kernel]  SPLower=%016lx SPUpper=%016lx\n", t->tc_SPLower, t->tc_SPUpper);
            
            if (((uint64_t *)regs.return_rsp < t->tc_SPLower)||((uint64_t *)regs.return_rsp > t->tc_SPUpper))
                rkprintf("[Kernel] Stack out of Bounds!\n");
        }
        
        rkprintf("[Kernel]  stack=%04x:%012x rflags=%016x ip=%04x:%012x err=%08x\n",
                 regs.return_ss, regs.return_rsp, regs.return_rflags, 
                 regs.return_cs, regs.return_rip, regs.error_code);

        rkprintf("[Kernel]  rax=%016lx rbx=%016lx rcx=%016lx rdx=%016lx\n", regs.rax, regs.rbx, regs.rcx, regs.rdx);
        rkprintf("[Kernel]  rsi=%016lx rdi=%016lx rbp=%016lx rsp=%016lx\n", regs.rsi, regs.rdi, regs.rbp, regs.return_rsp);
        rkprintf("[Kernel]  r08=%016lx r09=%016lx r10=%016lx r11=%016lx\n", regs.r8, regs.r9, regs.r10, regs.r11);
        rkprintf("[Kernel]  r12=%016lx r13=%016lx r14=%016lx r15=%016lx\n", regs.r12, regs.r13, regs.r14, regs.r15);
        rkprintf("[Kernel]  *rsp=%016lx\n", *(uint64_t *)regs.return_rsp);

        
        die = 1;
    }
    else if (regs.irq_number == 0x0e)        /* Page fault */
    {
        void *ptr = rdcr(cr2);
        unsigned char *ip = regs.return_rip;
        int i;
        
        die = 1;
        
        if (ptr == (void*)4)
        {
//            rkprintf("[Kernel] ** Code at %012lx is trying to access the SysBase at 4UL.\n", regs.return_rip);
            
            if (   (ip[0] & 0xfb) == 0x48 &&
                    ip[1]         == 0x8b && 
                   (ip[2] & 0xc7) == 0x04 &&
                    ip[3]         == 0x25
               )
            {
                int reg = ((ip[2] >> 3) & 0x07) | ((ip[0] & 0x04) << 1);
                
                switch(reg)
                {
                    case 0:
                        regs.rax = SysBase;
                        break;
                    case 1:
                        regs.rcx = SysBase;
                        break;
                    case 2:
                        regs.rdx = SysBase;
                        break;
                    case 3:
                        regs.rbx = SysBase;
                        break;
//                    case 4:   /* Cannot put SysBase into rSP register */
//                        regs.return_rsp = SysBase;
//                        break;
                    case 5:
                        regs.rbp = SysBase;
                        break;
                    case 6:
                        regs.rsi = SysBase;
                        break;
                    case 7:
                        regs.rdi = SysBase;
                        break;
                    case 8:
                        regs.r8 = SysBase;
                        break;
                    case 9:
                        regs.r9 = SysBase;
                        break;
                    case 10:
                        regs.r10 = SysBase;
                        break;
                    case 11:
                        regs.r11 = SysBase;
                        break;
                    case 12:
                        regs.r12 = SysBase;
                        break;
                    case 13:
                        regs.r13 = SysBase;
                        break;
                    case 14:
                        regs.r14 = SysBase;
                        break;
                    case 15:
                        regs.r15 = SysBase;
                        break;
                }
                
                regs.return_rip += 8;
                
                die = 0;
            }
            else if (   (ip[0] & 0xfb) == 0x48 &&
                    ip[1]         == 0x8b && 
                   (ip[2] & 0xc7) == 0x05
               )
            {
                int reg = ((ip[2] >> 3) & 0x07) | ((ip[0] & 0x04) << 1);
                
                switch(reg)
                {
                    case 0:
                        regs.rax = SysBase;
                        break;
                    case 1:
                        regs.rcx = SysBase;
                        break;
                    case 2:
                        regs.rdx = SysBase;
                        break;
                    case 3:
                        regs.rbx = SysBase;
                        break;
//                    case 4:   /* Cannot put SysBase into rSP register */
//                        regs.return_rsp = SysBase;
//                        break;
                    case 5:
                        regs.rbp = SysBase;
                        break;
                    case 6:
                        regs.rsi = SysBase;
                        break;
                    case 7:
                        regs.rdi = SysBase;
                        break;
                    case 8:
                        regs.r8 = SysBase;
                        break;
                    case 9:
                        regs.r9 = SysBase;
                        break;
                    case 10:
                        regs.r10 = SysBase;
                        break;
                    case 11:
                        regs.r11 = SysBase;
                        break;
                    case 12:
                        regs.r12 = SysBase;
                        break;
                    case 13:
                        regs.r13 = SysBase;
                        break;
                    case 14:
                        regs.r14 = SysBase;
                        break;
                    case 15:
                        regs.r15 = SysBase;
                        break;
                }
                
                regs.return_rip += 7;
                
                die = 0;
            }
        }
        
        if (die)
        {
            rkprintf("IRQ %02x:", regs.irq_number);
            rkprintf("[Kernel] PAGE FAULT EXCEPTION! %016p\n",ptr);

            if (t)
            {
                rkprintf("[Kernel]  %s %p '%s'\n",
                         t->tc_Node.ln_Type == NT_TASK?"task":"process", t, t->tc_Node.ln_Name);
                rkprintf("[Kernel]  SPLower=%016lx SPUpper=%016lx\n", t->tc_SPLower, t->tc_SPUpper);
                
                if (((uint64_t *)regs.return_rsp < t->tc_SPLower)||((uint64_t *)regs.return_rsp > t->tc_SPUpper))
                    rkprintf("[Kernel] Stack out of Bounds!\n");
            }

            rkprintf("[Kernel]  stack=%04x:%012x rflags=%016x ip=%04x:%012x err=%08x\n",
                     regs.return_ss, regs.return_rsp, regs.return_rflags, 
                     regs.return_cs, regs.return_rip, regs.error_code);
            
            rkprintf("[Kernel]  rax=%016lx rbx=%016lx rcx=%016lx rdx=%016lx\n", regs.rax, regs.rbx, regs.rcx, regs.rdx);
            rkprintf("[Kernel]  rsi=%016lx rdi=%016lx rbp=%016lx rsp=%016lx\n", regs.rsi, regs.rdi, regs.rbp, regs.return_rsp);
            rkprintf("[Kernel]  r08=%016lx r09=%016lx r10=%016lx r11=%016lx\n", regs.r8, regs.r9, regs.r10, regs.r11);
            rkprintf("[Kernel]  r12=%016lx r13=%016lx r14=%016lx r15=%016lx\n", regs.r12, regs.r13, regs.r14, regs.r15);
            rkprintf("[Kernel]  *rsp=%016lx\n", *(uint64_t *)regs.return_rsp);
            
            rkprintf("[Kernel]  Insn: ");
            for (i = 0; i < 16; i++)
                rkprintf("%02x ", ip[i]);
            rkprintf("\n");

        }
    }
    else if (regs.irq_number == 0x80) /* Syscall? */
    {
        switch (regs.rax)
        {
            case SC_CAUSE:
                core_Cause(SysBase);
                break;

            case SC_DISPATCH:
                core_Dispatch(&regs);
                break;
                
            case SC_SWITCH:
                core_Switch(&regs);
                break;
            
            case SC_SCHEDULE:
                if (regs.ds != KERNEL_DS)
                    core_Schedule(&regs);
                break;
        }
    }
    
    if (KernelBase)
    {
        if (KernelBase->kb_Intr[regs.irq_number].lh_Type == KBL_APIC)
            core_APIC_AckIntr(regs.irq_number);
        
        if (KernelBase->kb_Intr[regs.irq_number].lh_Type == KBL_XTPIC)
            core_XTPIC_AckIntr(regs.irq_number);
        
        
        if (!IsListEmpty(&KernelBase->kb_Intr[regs.irq_number]))
        {
            struct IntrNode *in, *in2;

            ForeachNodeSafe(&KernelBase->kb_Intr[regs.irq_number], in, in2)
            {
                if (in->in_Handler)
                    in->in_Handler(in->in_HandlerData, in->in_HandlerData2);
            }
            
            if (KernelBase->kb_Intr[regs.irq_number].lh_Type == KBL_XTPIC)
                core_XTPIC_EnableIRQ(regs.irq_number);
        }
    }
    
    while (die) asm volatile ("hlt");
}

asm(".text\n\t"
    ".globl core_DefaultIRETQ\n\t"
    ".type core_DefaultIRETQ,@function\n"
"core_DefaultIRETQ: iretq");
