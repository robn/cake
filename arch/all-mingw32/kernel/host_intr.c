#define DEBUG 0

#include <aros/system.h>
#include <windows.h>
#define __typedef_LONG /* LONG, ULONG, WORD, BYTE and BOOL are declared in Windows headers. Looks like everything  */
#define __typedef_WORD /* is the same except BOOL. It's defined to short on AROS and to int on Windows. This means */
#define __typedef_BYTE /* that you can't use it in OS-native part of the code and can't use any AROS structure     */
#define __typedef_BOOL /* definition that contains BOOL.                                                           */
typedef unsigned AROS_16BIT_TYPE UWORD;
typedef unsigned char UBYTE;

#include <stddef.h>
#include <stdio.h>
#include <exec/lists.h>
#include <exec/execbase.h>
#include "kernel_intern.h"
#include "syscall.h"
#include "host_debug.h"
#include "cpucontext.h"

#define DI(x)   /* Interrupts debug     */
#define DS(x)   /* Task switcher debug  */
#define DIRQ(x) /* IRQ debug		*/

#define AROS_EXCEPTION_SYSCALL 0x80000001

struct SwitcherData {
    HANDLE MainThread;
    HANDLE IntObjects[INTERRUPTS_NUM];
};

struct SwitcherData SwData;
DWORD *LastErrorPtr;
unsigned char Ints_Enabled;
unsigned char PendingInts[INTERRUPTS_NUM];
unsigned char Supervisor;
unsigned char Sleep_Mode;
struct ExecBase **SysBasePtr;
struct KernelBase **KernelBasePtr;

void user_handler(uint8_t exception, struct List *list)
{
    if (!IsListEmpty(&list[exception]))
    {
        struct IntrNode *in, *in2;

        ForeachNodeSafe(&list[exception], in, in2)
        {
            if (in->in_Handler)
                in->in_Handler(in->in_HandlerData, in->in_HandlerData2);
        }
    }
}

LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS Except)
{
    	struct ExecBase *SysBase = *SysBasePtr;
    	struct KernelBase *KernelBase = *KernelBasePtr;
    	REG_SAVE_VAR;

	Supervisor++;
	switch (Except->ExceptionRecord->ExceptionCode) {
	case AROS_EXCEPTION_SYSCALL:
	    CONTEXT_SAVE_REGS(Except->ContextRecord);
	    DI(printf("[KRN] Syscall exception %lu\n", *Except->ExceptionRecord->ExceptionInformation));
	    switch (*Except->ExceptionRecord->ExceptionInformation)
	    {
	    case SC_CAUSE:
	        core_Cause(SysBase);
	        break;
	    case SC_DISPATCH:
	        core_Dispatch(Except->ContextRecord);
	        break;
	    case SC_SWITCH:
	        core_Switch(Except->ContextRecord);
	        break;
	    case SC_SCHEDULE:
	        core_Schedule(Except->ContextRecord);
	        break;
	    }
	    CONTEXT_RESTORE_REGS(Except->ContextRecord);
	    Supervisor--;
	    return EXCEPTION_CONTINUE_EXECUTION;
	default:
	    printf("[KRN] Exception 0x%08lX handler. Context @ %p, SysBase @ %p, KernelBase @ %p\n", Except->ExceptionRecord->ExceptionCode, Except->ContextRecord, SysBase, KernelBase);
    	    if (SysBase)
    	    {
        	struct Task *t = SysBase->ThisTask;
        	
        	if (t)
        	    printf("[KRN] %s %p (%s)\n", t->tc_Node.ln_Type == NT_TASK ? "Task":"Process", t, t->tc_Node.ln_Name ? t->tc_Node.ln_Name : "--unknown--");
        	else
        	    printf("[KRN] No task\n");
    	    }
    	    PRINT_CPUCONTEXT(Except->ContextRecord);
    	    printf("[KRN] **UNHANDLED EXCEPTION** stopping here...\n");
	    return EXCEPTION_EXECUTE_HANDLER;
	}
}

DWORD WINAPI TaskSwitcher(struct SwitcherData *args)
{
    HANDLE IntEvent;
    DWORD obj;
    CONTEXT MainCtx;
    REG_SAVE_VAR;
    DS(DWORD res);
    MSG msg;

    for (;;) {
        obj = WaitForMultipleObjects(INTERRUPTS_NUM, args->IntObjects, FALSE, INFINITE);
        DS(bug("[Task switcher] Object %lu signalled\n", obj));
        if (Sleep_Mode != SLEEP_MODE_ON) {
            DS(res =) SuspendThread(args->MainThread);
    	    DS(bug("[Task switcher] Suspend thread result: %lu\n", res));
    	}
        if (Ints_Enabled) {
    	    Supervisor++;
    	    PendingInts[obj] = 0;
    	    /* 
    	     * We will get and store the complete CPU context, but set only part of it.
    	     * This can be a useful aid for future AROS debuggers.
    	     */
    	    CONTEXT_INIT_FLAGS(&MainCtx);
    	    DS(res =) GetThreadContext(args->MainThread, &MainCtx);
    	    DS(bug("[Task switcher] Get context result: %lu\n", res));
    	    CONTEXT_SAVE_REGS(&MainCtx);
    	    DS(OutputDebugString("[Task switcher] original CPU context: ****\n"));
    	    DS(PrintCPUContext(&MainCtx));
    	    if (*KernelBasePtr)
	    	user_handler(obj, (*KernelBasePtr)->kb_Interrupts);
    	    core_ExitInterrupt(&MainCtx);
    	    if (!Sleep_Mode) {
    	        DS(OutputDebugString("[Task switcher] new CPU context: ****\n"));
    	        DS(PrintCPUContext(&MainCtx));
    	        CONTEXT_RESTORE_REGS(&MainCtx);
    	        DS(res =)SetThreadContext(args->MainThread, &MainCtx);
    	        DS(bug("[Task switcher] Set context result: %lu\n", res));
    	    }
    	    Supervisor--;
    	} else {
    	    PendingInts[obj] = 1;
            DS(bug("[KRN] Interrupts are disabled, interrupt %lu is pending\n", obj));
        }
        if (Sleep_Mode)
            /* We've entered sleep mode */
            Sleep_Mode = SLEEP_MODE_ON;
        else {
            DS(res =) ResumeThread(args->MainThread);
            DS(bug("[Task switcher] Resume thread result: %lu\n", res));
        }
    }
    return 0;
}

/* ****** Interface functions ****** */

long __declspec(dllexport) core_intr_disable(void)
{
    DI(printf("[KRN] disabling interrupts\n"));
    Ints_Enabled = 0;
}

long __declspec(dllexport) core_intr_enable(void)
{
    int i;

    DI(printf("[KRN] enabling interrupts\n"));
    Ints_Enabled = 1;
    /* FIXME: here we do not force timer interrupt, probably this is wrong. However there's no way
       to force-trigger a waitable timer in Windows. A workaround is possible, but the design will
       be complicated then (we need a companion event in this case). Probably it will be implemented
       in future. */
    for (i = INT_IO; i < INTERRUPTS_NUM; i++) {
        if (PendingInts[i]) {
            DI(printf("[KRN] enable: sigalling about pending interrupt %lu\n", i));
            SetEvent(SwData.IntObjects[i]);
        }
    }
}

void __declspec(dllexport) core_syscall(unsigned long n)
{
    RaiseException(AROS_EXCEPTION_SYSCALL, 0, 1, &n);
    /* If after RaiseException we are still here, but Sleep_Mode != 0, this likely means
       we've just called SC_SCHEDULE, SC_SWITCH or SC_DISPATCH, and it is putting us to sleep.
       Sleep mode will be committed as soon as timer IRQ happens */
    while(Sleep_Mode) {
    	/* TODO: SwitchToThread() here maybe? But it's dangerous because context switch
    	   will happen inside it and Windows will kill us */
    }
}

unsigned char __declspec(dllexport) core_is_super(void)
{
    return Supervisor;
}

BOOL InitIntObjects(HANDLE *Objs)
{
    int i;

    for (i = 0; i < INTERRUPTS_NUM; i++) {
        Objs[i] = NULL;
        PendingInts[i] = 0;
    }
    /* Timer interrupt is a waitable timer, it's not an event */
    for (i = INT_IO; i < INTERRUPTS_NUM; i++) {
        Objs[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!Objs[i])
            return FALSE;
    }
    return TRUE;
}

void CleanupIntObjects(HANDLE *Objs)
{
    int i;

    for (i = 0; i < INTERRUPTS_NUM; i++) {
        if (Objs[i])
            CloseHandle(Objs[i]);
    }
}

int __declspec(dllexport) core_init(unsigned long TimerPeriod, struct ExecBase **SysBasePointer, struct KernelBase **KernelBasePointer)
{
    HANDLE ThisProcess;
    HANDLE SwitcherThread;
    LARGE_INTEGER VBLPeriod;
    OSVERSIONINFO osver;
    void *MainTEB;
    int i;
    DWORD SwitcherId;
    ULONG LastErrOffset = 0;

    D(printf("[KRN] Setting up interrupts, SysBasePtr = 0x%08lX, KernelBasePtr = 0x%08lX\n", SysBasePointer, KernelBasePointer));
    SysBasePtr = SysBasePointer;
    KernelBasePtr = KernelBasePointer;
    Ints_Enabled = 0;
    Supervisor = 0;
    Sleep_Mode = 0;
    SetUnhandledExceptionFilter(ExceptionHandler);
    if (InitIntObjects(SwData.IntObjects)) {
    	SwData.IntObjects[INT_TIMER] = CreateWaitableTimer(NULL, 0, NULL);
    	if (SwData.IntObjects[INT_TIMER]) {
	    ThisProcess = GetCurrentProcess();
	    if (DuplicateHandle(ThisProcess, GetCurrentThread(), ThisProcess, &SwData.MainThread, 0, TRUE, DUPLICATE_SAME_ACCESS)) {
	        FillMemory(&osver, sizeof(osver), 0);
	        osver.dwOSVersionInfoSize = sizeof(osver);
	        GetVersionEx(&osver);
	        /* LastError value is part of our context. In order to manipulate it we have to hack
	           into Windows TEB (thread environment block).
	           Since this structure is private, error code offset changes from version to version.
	           The following offsets are known:
	           * Windows 95 and 98 - 0x60
	           * Windows Me - 0x74
	           * Windows NT (all family, fixed at last) - 0x34
	         */
	        switch(osver.dwPlatformId) {
	        case VER_PLATFORM_WIN32_WINDOWS:
	            if (osver.dwMajorVersion == 4) {
	                if (osver.dwMinorVersion > 10)
	                    LastErrOffset = 0x74;
	                else
	                    LastErrOffset = 0x60;
	            }
	            break;
	        case VER_PLATFORM_WIN32_NT:
	            LastErrOffset = 0x34;
	            break;
	        }
	        if (LastErrOffset) {
		    MainTEB = NtCurrentTeb();
		    LastErrorPtr = MainTEB + LastErrOffset;
		    SwitcherThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TaskSwitcher, &SwData, 0, &SwitcherId);
		    if (SwitcherThread) {
			D(printf("[KRN] Task switcher started, ID %lu\n", SwitcherId));
#ifdef SLOW
			TimerPeriod = 5000;
#else
			TimerPeriod = 1000/TimerPeriod;
#endif
			VBLPeriod.QuadPart = -10000*(LONGLONG)TimerPeriod;
			return SetWaitableTimer(SwData.IntObjects[INT_TIMER], &VBLPeriod, TimerPeriod, NULL, NULL, 0);
		    }
			D(else printf("[KRN] Failed to run task switcher thread\n");)
		} else
		    printf("Unsupported Windows version %u.%u, platform ID %u\n", osver.dwMajorVersion, osver.dwMinorVersion, osver.dwPlatformId);
	    }
		D(else printf("[KRN] failed to get thread handle\n");)
	}
	    D(else printf("[KRN] Failed to create timer interrupt\n");)
    }
        D(else printf("[KRN] failed to create interrupt objects\n");)
    CleanupIntObjects(SwData.IntObjects);
    return 0;
}

/*
 * This is the only function to be called by modules other than kernel.resource.
 * It is used for causing interrupts from within asynchronous threads of
 * emul.handler and wingdi.hidd.
 */

unsigned long __declspec(dllexport) KrnCauseIRQ(unsigned char irq)
{
    unsigned long res;

    D(printf("[kernel IRQ] Causing IRQ %u\n", irq));
    res = SetEvent(SwData.IntObjects[irq]);
    D(printf("[kernel IRQ] Result: %ld\n", res));
    return res;
}
