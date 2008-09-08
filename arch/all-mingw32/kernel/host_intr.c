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

#define DS(x) x
#define SLOW

#define AROS_EXCEPTION_SYSCALL 0x80000001

struct SwitcherData {
    HANDLE MainThread;
    HANDLE IntTimer;
};

struct SwitcherData SwData;
unsigned char Ints_Enabled = 0;
struct ExecBase **SysBasePtr;
struct KernelBase **KernelBasePtr;

void user_handler(uint8_t exception)
{
    struct KernelBase *KernelBase = *KernelBasePtr;

    if (KernelBase) {
        if (!IsListEmpty(&KernelBase->kb_Exceptions[exception]))
        {
            struct IntrNode *in, *in2;

            ForeachNodeSafe(&KernelBase->kb_Exceptions[exception], in, in2)
            {
                if (in->in_Handler)
                    in->in_Handler(in->in_HandlerData, in->in_HandlerData2);
            }
        }
    }
    
}

LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS Except)
{
    	struct ExecBase *SysBase = *SysBasePtr;
    	struct KernelBase *KernelBase = *KernelBasePtr;

	switch (Except->ExceptionRecord->ExceptionCode) {
	case AROS_EXCEPTION_SYSCALL:
	    D(printf("[KRN] Syscall exception %lu\n", *Except->ExceptionRecord->ExceptionInformation));
	    switch (*Except->ExceptionRecord->ExceptionInformation)
	    {
	    case SC_CAUSE:
	        if (SysBase)
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
	    return EXCEPTION_CONTINUE_EXECUTION;
	default:
	    printf("[KRN] Exception 0x%08lX handler. Context @ %p, SysBase @ %p, KernelBase @ %p\n", Except->ExceptionRecord->ExceptionCode, Except->ContextRecord, SysBase, KernelBase);
    	    if (SysBase)
    	    {
        	struct Task *t = SysBase->ThisTask;
        	printf("[KRN] %s %p (%s)\n", t->tc_Node.ln_Type == NT_TASK ? "Task":"Process", t, t->tc_Node.ln_Name ? t->tc_Node.ln_Name : "--unknown--");
    	    }
	    printf("[KRN] **UNHANDLED EXCEPTION** stopping here...\n");
	    return EXCEPTION_EXECUTE_HANDLER;
	}
}

DWORD TaskSwitcher(struct SwitcherData *args)
{
    HANDLE IntEvent;
    DWORD obj;
    CONTEXT MainCtx;

    for (;;) {
        WaitForSingleObject(args->IntTimer, INFINITE);
        DS(printf("[KRN] Timer interrupt\n"));
    	SuspendThread(args->MainThread);
    	if (Ints_Enabled) {
    	    GetThreadContext(args->MainThread, &MainCtx);
    	    user_handler(0);
    	    core_ExitInterrupt(&MainCtx);
    	    SetThreadContext(args->MainThread, &MainCtx);
    	}
            DS(else printf("[KRN] Interrupts are disabled\n"));
        ResumeThread(args->MainThread);
    }
    return 0;
}

/* ****** Interface functions ****** */

long __declspec(dllexport) core_intr_disable(void)
{
    D(printf("[KRN] disabling interrupts\n"));
    Ints_Enabled = 0;
}

long __declspec(dllexport) core_intr_enable(void)
{
    D(printf("[KRN] enabling interrupts\n"));
    Ints_Enabled = 1;
}

void __declspec(dllexport) core_syscall(unsigned long n)
{
    RaiseException(AROS_EXCEPTION_SYSCALL, 0, 1, &n);
}

int __declspec(dllexport) core_init(unsigned long TimerPeriod, struct ExecBase **SysBasePointer, struct KernelBase **KernelBasePointer)
{
    HANDLE ThisProcess;
    HANDLE SwitcherThread;
    DWORD SwitcherId;
    LARGE_INTEGER VBLPeriod;

    D(printf("[KRN] Setting up interrupts, SysBasePtr = 0x%08lX, KernelBasePtr = 0x%08lX\n", SysBasePointer, KernelBasePointer));
    SysBasePtr = SysBasePointer;
    KernelBasePtr = KernelBasePointer;
    SetUnhandledExceptionFilter(ExceptionHandler);
    SwData.IntTimer = CreateWaitableTimer(NULL, 0, NULL);
    if (SwData.IntTimer) {
	ThisProcess = GetCurrentProcess();
	if (DuplicateHandle(ThisProcess, GetCurrentThread(), ThisProcess, &SwData.MainThread, 0, TRUE, DUPLICATE_SAME_ACCESS)) {
	    SwitcherThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TaskSwitcher, &SwData, 0, &SwitcherId);
	    if (SwitcherThread) {
	  	D(printf("[KRN] Task switcher started\n"));
	  	CloseHandle(SwitcherThread);
#ifdef SLOW
		TimerPeriod = 5000;
#else
		TimerPeriod = 1000/TimerPeriod;
#endif
		VBLPeriod.QuadPart = -10000*(LONGLONG)TimerPeriod;
	  	return SetWaitableTimer(SwData.IntTimer, &VBLPeriod, TimerPeriod, NULL, NULL, 0);
	    }
	        D(else printf("[KRN] Failed to run task switcher thread\n");)
	}
	    D(else printf("[KRN] failed to get thread handle\n");)
    }
        D(else printf("[KRN] failed to create VBlank timer\n");)
    return 0;
}
