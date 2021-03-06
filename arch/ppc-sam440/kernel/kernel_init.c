#include <aros/kernel.h>
#include <aros/libcall.h>
#include <aros/symbolsets.h>
#include <inttypes.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <utility/tagitem.h>
#include <asm/amcc440.h>
#include <asm/io.h>
#include <strings.h>

#include <proto/exec.h>

#include "kernel_intern.h"
#include LC_LIBDEFS_FILE
#include "syscall.h"

static void __attribute__((used)) kernel_cstart(struct TagItem *msg);
int exec_main(struct TagItem *msg, void *entry);

/* A very very very.....
 * ... very ugly code.
 * 
 * The AROS kernel gets executed at this place. The stack is unknown here, might be
 * set properly up, might be totally broken aswell and thus one cannot trust the contents
 * of %r1 register. Even worse, the kernel has been relocated most likely to some virtual 
 * address and the MMU mapping might be not ready now.
 * 
 * The strategy is to create one MMU entry first, mapping first 16MB of ram into last 16MB 
 * of address space in one turn and then making proper MMU map once the bss sections are cleared
 * and the startup routine in C is executed. This "trick" assume two evil things:
 * - the kernel will be loaded (and will fit completely) within first 16MB of RAM, and
 * - the kernel will be mapped into top (last 16MB) of memory.
 * 
 * Yes, I'm evil ;) 
 */ 
 
asm(".section .aros.init,\"ax\"\n\t"
    ".globl start\n\t"
    ".type start,@function\n"
    "start:\n\t"
    "mr %r29,%r3\n\t"           /* Don't forget the message */
    "lis %r9,0xff00\n\t"        /* Virtual address 0xff000000 */
    "li %r10,0\n\t"             /* Physical address 0x00000000 */
    "ori %r9,%r9,0x0270\n\t"    /* 16MB page. Valid one */
    "li %r11,0x043f\n\t"        /* Write through cache. RWX enabled :) */
    "li %r0,0\n\t"              /* TLB entry number 0 */
    "tlbwe %r9,%r0,0\n\t"
    "tlbwe %r10,%r0,1\n\t"
    "tlbwe %r11,%r0,2\n\t"
    "isync\n\t"                         /* Invalidate shadow TLB's */
    "li %r9,0; mtspr 0x114,%r9; mtspr 0x115,%r9; mttbl %r9; mttbu %r9; mttbl %r9\n\t"
    "lis %r9,tmp_stack_end@ha\n\t"      /* Use temporary stack while clearing BSS */
    "lwz %r1,tmp_stack_end@l(%r9)\n\t"
    "bl __clear_bss\n\t"                /* Clear 'em ALL!!! */
    "lis %r11,target_address@ha\n\t"    /* Load the address of init code in C */
    "mr %r3,%r29\n\t"                   /* restore the message */
    "lwz %r11,target_address@l(%r11)\n\t"
    "lis %r9,stack_end@ha\n\t"          /* Use brand new stack to do evil things */
    "mtctr %r11\n\t"
    "lwz %r1,stack_end@l(%r9)\n\t"
    "bctr\n\t"                          /* And start the game... */
    ".string \"Native/CORE v3 (" __DATE__ ")\""
    "\n\t.text\n\t"
);

static void __attribute__((used)) __clear_bss(struct TagItem *msg) 
{
    struct KernelBSS *bss;
    
    bss =(struct KernelBSS *)krnGetTagData(KRN_KernelBss, 0, msg);

    if (bss)
    {
        while (bss->addr && bss->len)
        {
            bzero(bss->addr, bss->len);
            bss++;
        }   
    }
}

static __attribute__((used,section(".data"),aligned(16))) union {
    struct TagItem bootup_tags[64];
    uint32_t  tmp_stack[128];
} tmp_struct;
static const uint32_t *tmp_stack_end __attribute__((used, section(".text"))) = &tmp_struct.tmp_stack[124];
static uint32_t stack[STACK_SIZE] __attribute__((used,aligned(16)));
static uint32_t stack_super[STACK_SIZE] __attribute__((used,aligned(16)));
static const uint32_t *stack_end __attribute__((used, section(".text"))) = &stack[STACK_SIZE-4];
static const void *target_address __attribute__((used, section(".text"))) = (void*)kernel_cstart;
static struct TagItem *BootMsg;

static void __attribute__((used)) kernel_cstart(struct TagItem *msg)
{
    int i;
    uint32_t v1,v2,v3;

    /* Disable interrupts and let FPU work */
    wrmsr((rdmsr() & ~(MSR_CE | MSR_EE | MSR_ME)) | MSR_FP);
        
    /* Enable FPU */
    wrspr(CCR0, rdspr(CCR0) & ~0x00100000);
    wrspr(CCR1, rdspr(CCR1) | (0x80000000 >> 24));
    
    wrspr(SPRG4, 0);    /* Clear KernelBase */
    wrspr(SPRG5, 0);    /* Clear SysBase */

    D(bug("[KRN] Kernel resource pre-exec init\n"));
    D(bug("[KRN] MSR=%08x CRR0=%08x CRR1=%08x\n", rdmsr(), rdspr(CCR0), rdspr(CCR1)));
    D(bug("[KRN] USB config %08x\n", rddcr(SDR0_USB0)));
    
    wrspr(SPRG0, (uint32_t)&stack_super[STACK_SIZE-4]);
    
    /* Do a slightly more sophisticated MMU map */
    mmu_init(msg);
    intr_init();
    
    /* 
     * Slow down the decrement interrupt a bit. Rough guess is that UBoot has left us with
     * 1kHz DEC counter.
     */
    wrspr(DECAR, 0xffffffff);
    
    /* Copy the boot message */
    
    
    
    /* Start exec.library up */
    
    exec_main(msg, NULL);
    //exec_main(BootMsg, NULL);
    
    asm volatile("li %%r3,%0; sc"::"i"(SC_SUPERSTATE):"memory","r3");
    /* 
     * Do never ever try to return. THis coude would attempt to go back to the physical address
     * of asm trampoline, not the virtual one!
     */
    while(1) {
        wrmsr(rdmsr() | MSR_POW);
    }
}

AROS_LH0I(struct TagItem *, KrnGetBootInfo,
         struct KernelBase *, KernelBase, 10, Kernel)
{
    AROS_LIBFUNC_INIT

    return BootMsg;

    AROS_LIBFUNC_EXIT
}


static int Kernel_Init(LIBBASETYPEPTR LIBBASE)
{
    int i;
    struct ExecBase *SysBase = getSysBase();
    
    /* 
     * Set the KernelBase into SPRG4. At this stage the SPRG5 should be already set by
     * exec.library itself.
     */
    wrspr(SPRG4, LIBBASE);

    D(bug("[KRN] Kernel resource post-exec init\n"));

    D(bug("[KRN] Allowing userspace to flush caches\n"));
    wrspr(MMUCR,rdspr(MMUCR) & ~0x000c0000);
    
    for (i=0; i < 16; i++)
        NEWLIST(&LIBBASE->kb_Exceptions[i]);

    for (i=0; i < 64; i++)
        NEWLIST(&LIBBASE->kb_Interrupts[i]);

    /* Prepare private memory block */
    LIBBASE->kb_SupervisorMem = (struct MemHeader *)0xff000000;
    
    /* 
     * kernel.resource is ready to run. Enable external interrupts and leave 
     * supervisor mode
     */
    wrmsr(rdmsr() | (MSR_EE|MSR_FP));
    D(bug("[KRN] Interrupts enabled\n"));
    
    wrmsr(rdmsr() | (MSR_PR));
    D(bug("[KRN] Entered user mode \n"));
    

    Permit();

}

ADD2INITLIB(Kernel_Init, 0)
