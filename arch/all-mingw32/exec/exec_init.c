/*
    Copyright  1995-2009, The AROS Development Team. All rights reserved.
    $Id$

    Desc: exec.library resident and initialization.
    Lang: english
*/
#define DEBUG 0

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/execbase.h>
#include <exec/interrupts.h>
#include <exec/resident.h>
#include <exec/memory.h>
#include <exec/alerts.h>
#include <exec/tasks.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <dos/dosextens.h>

#include <aros/system.h>
#include <aros/arossupportbase.h>
#include <aros/asmcall.h>
#include <aros/config.h>

#include <aros/debug.h>

#include <proto/arossupport.h>
#include <proto/exec.h>
#include <proto/kernel.h>
#include <clib/macros.h> /* need ABS() */

#include "exec_intern.h"
#include "exec_util.h"
#include "etask.h"
#include LC_LIBDEFS_FILE

static const UBYTE name[];
static const UBYTE version[];
extern const char LIBEND;
struct ExecBase *GM_UNIQUENAME(init)();

const struct Resident Exec_resident =
{
    RTC_MATCHWORD,
    (struct Resident *)&Exec_resident,
    (APTR)&LIBEND,
    RTF_SINGLETASK,
    VERSION_NUMBER,
    NT_LIBRARY,
    105,
    (STRPTR)name,
    (STRPTR)&version[6],
    &GM_UNIQUENAME(init)
};

static const UBYTE name[] = MOD_NAME_STRING;
static const UBYTE version[] = VERSION_STRING;

extern void debugmem(void);

/**************** GLOBAL SYSBASE ***************/
struct ExecBase * SysBase = NULL;

/*
    We temporarily redefine kprintf() so we use the real version in case
    we have one of these two fn's called before AROSSupportBase is ready.
*/

#undef kprintf
#undef rkprintf
#undef vkprintf
struct Library * PrepareAROSSupportBase (struct ExecBase * SysBase)
{
	struct AROSSupportBase * AROSSupportBase;
	AROSSupportBase = AllocMem(sizeof(struct AROSSupportBase), MEMF_CLEAR);
	AROSSupportBase->kprintf = (void *)kprintf;
	AROSSupportBase->rkprintf = (void *)rkprintf;
	AROSSupportBase->vkprintf = (void *)vkprintf;
    	NEWLIST(&AROSSupportBase->AllocMemList);

#warning FIXME Add code to read in the debug options

	return (struct Library *)AROSSupportBase;
}

void _aros_not_implemented(char *X)
{
    kprintf("Unsupported function at offset -0x%h in %s\n",
	    ABS(*(WORD *)((&X)[-1]-2)),
	    ((struct Library *)(&X)[-2])->lib_Node.ln_Name);
}
#define kprintf (((struct AROSSupportBase *)(SysBase->DebugAROSBase))->kprintf)
#define rkprintf (((struct AROSSupportBase *)(SysBase->DebugAROSBase))->rkprintf)
#define vkprintf (((struct AROSSupportBase *)(SysBase->DebugAROSBase))->vkprintf)

/* IntServer:
    This interrupt handler will send an interrupt to a series of queued
    interrupt servers. Servers should return D0 != 0 (Z clear) if they
    believe the interrupt was for them, and no further interrupts will
    be called. This will only check the value in D0 for non-m68k systems,
    however it SHOULD check the Z-flag on 68k systems.

    Hmm, in that case I would have to separate it from this file in order
    to replace it...
*/
AROS_UFH5S(void, IntServer,
    AROS_UFHA(ULONG, intMask, D0),
    AROS_UFHA(struct Custom *, custom, A0),
    AROS_UFHA(struct List *, intList, A1),
    AROS_UFHA(APTR, intCode, A5),
    AROS_UFHA(struct ExecBase *, SysBase, A6))
{
    AROS_USERFUNC_INIT

    struct Interrupt * irq;

    ForeachNode(intList, irq)
    {
	if( AROS_UFC4(int, irq->is_Code,
		AROS_UFCA(struct Custom *, custom, A0),
		AROS_UFCA(APTR, irq->is_Data, A1),
		AROS_UFCA(APTR, irq->is_Code, A5),
		AROS_UFCA(struct ExecBase *, SysBase, A6)
	))
	    break;
    }

    AROS_USERFUNC_EXIT
}

void VBlankHandler(struct ExecBase *SysBase, void *dummy)
{
    struct IntVector *iv = &SysBase->IntVects[INTB_VERTB];

    /* First decrease Elapsed time for current task */
    if (SysBase->Elapsed && (--SysBase->Elapsed == 0))
    {
        SysBase->SysFlags |= 0x2000;
        SysBase->AttnResched |= ARF_AttnSwitch;
    }
    
    /* If the VBlank vector in SysBase is set, call it. */
    if (iv->iv_Code)
    {
         AROS_UFC5(void, iv->iv_Code,
            AROS_UFCA(ULONG, 0, D1),
            AROS_UFCA(ULONG, 0, A0),
            AROS_UFCA(APTR, iv->iv_Data, A1),
            AROS_UFCA(APTR, iv->iv_Code, A5),
            AROS_UFCA(struct ExecBase *, SysBase, A6)
        );
    }
}

extern ULONG SoftIntDispatch();

AROS_UFH3(LIBBASETYPEPTR, GM_UNIQUENAME(init),
    AROS_UFHA(ULONG, dummy, D0),
    AROS_UFHA(BPTR, segList, A0),
    AROS_UFHA(struct ExecBase *, sysBase, A6)
)
{
    AROS_USERFUNC_INIT

    D(bug("[exec_init] Entered exec.library init\n"));
    SysBase = sysBase;
    /*
	Create boot task.  Sigh, we actually create a Process sized Task,
	since DOS needs to call things which think it has a Process and
	we don't want to overwrite memory with something strange do we?

	We do this until at least we can boot dos more cleanly.
    */
    {
	struct Task    *t;
	struct MemList *ml;

	ml = (struct MemList *)AllocMem(sizeof(struct MemList), MEMF_PUBLIC|MEMF_CLEAR);
	t  = (struct Task *)   AllocMem(sizeof(struct Process), MEMF_PUBLIC|MEMF_CLEAR);

	if( !ml || !t )
	{
	    kprintf("ERROR: Cannot create Boot Task!\n");
	    Alert( AT_DeadEnd | AG_NoMemory | AN_ExecLib );
	}
	ml->ml_NumEntries = 1;
	ml->ml_ME[0].me_Addr = t;
	ml->ml_ME[0].me_Length = sizeof(struct Process);

	NEWLIST(&t->tc_MemEntry);
	NEWLIST(&((struct Process *)t)->pr_MsgPort.mp_MsgList);

	/* It's the boot process that RunCommand()s the boot shell, so we
	   must have this list initialized */
	NEWLIST((struct List *)&((struct Process *)t)->pr_LocalVars);

	AddHead(&t->tc_MemEntry,&ml->ml_Node);

	t->tc_Node.ln_Name = "Boot Task";
	t->tc_Node.ln_Pri = 0;
	t->tc_State = TS_RUN;
	t->tc_SigAlloc = 0xFFFF;
	t->tc_SPLower = 0;	    /* This is the system's stack */
	t->tc_SPUpper = (APTR)~0UL;
	t->tc_Flags |= TF_ETASK;

	t->tc_UnionETask.tc_ETask = AllocVec
	(
	    sizeof(struct IntETask),
	    MEMF_ANY|MEMF_CLEAR
	);

	if (!t->tc_UnionETask.tc_ETask)
	{
	    kprintf("Not enough memory for first task\n");
	    Alert( AT_DeadEnd | AG_NoMemory | AN_ExecLib );
	}

	/* Initialise the ETask data. */
	InitETask(t, t->tc_UnionETask.tc_ETask);

	GetIntETask(t)->iet_Context = KrnCreateContext();
	if (!GetIntETask(t)->iet_Context)
	{
	    kprintf("Not enough memory for first task\n");
	    Alert( AT_DeadEnd | AG_NoMemory | AN_ExecLib );
	}

	sysBase->ThisTask = t;
	sysBase->Elapsed = sysBase->Quantum;
    }
    {
	/* Install the interrupt servers */
	int i;
	for(i=0; i < 16; i++)
	    if( (1<<i) & (INTF_PORTS|INTF_COPER|INTF_VERTB|INTF_EXTER|INTF_SETCLR))
	    {
		struct Interrupt *is;
		struct SoftIntList *sil;
		is = AllocMem(sizeof(struct Interrupt) + sizeof(struct SoftIntList),
				MEMF_CLEAR|MEMF_PUBLIC);
		if( is == NULL )
		{
		    kprintf("ERROR: Cannot install Interrupt Servers!\n");
		    Alert( AT_DeadEnd | AN_IntrMem );
		}
		sil = (struct SoftIntList *)((struct Interrupt *)is + 1);

		is->is_Code = &IntServer;
		is->is_Data = sil;
		NEWLIST((struct List *)sil);
		SetIntVector(i,is);
	    }
	    else
	    {
	      struct Interrupt * is;
	      switch(i)
	      {
	        case INTB_SOFTINT:
	          is = AllocMem(sizeof(struct Interrupt), MEMF_CLEAR|MEMF_PUBLIC);
	          if (NULL == is)
	          {
	            kprintf("Error: Cannot install Interrupt Handler!\n");
	            Alert( AT_DeadEnd | AN_IntrMem );
	          }
	          is->is_Node.ln_Type = NT_INTERRUPT;
	          is->is_Node.ln_Pri = 0;
	          is->is_Node.ln_Name = "SW Interrupt Dispatcher";
	          is->is_Data = NULL;
	          is->is_Code = (void *)SoftIntDispatch;
	          SetIntVector(i,is);
	        break;
	      }
	    }
    }
    /* Install the VBlank handler. We drop the handle because exec.library never expunges. */
    KrnAddIRQHandler(0, VBlankHandler, sysBase, NULL);

    /* We now start up the interrupts */
    Permit();
    Enable();
    
    D(debugmem());

    /* This will cause everything else to run. This call will not return.
	This is because it eventually falls into strap, which will call
	the bootcode, which itself is not supposed to return. It is up
	to the DOS (whatever it is) to Permit(); RemTask(NULL);
    */
    InitCode(RTF_COLDSTART, 0);

    /* There had better be some kind of task waiting to run. */
    return NULL;
    AROS_USERFUNC_EXIT
}

#ifdef sysBase
#undef sysBase
#endif

AROS_PLH1(struct ExecBase *, open,
    AROS_LHA(ULONG, version, D0),
    struct ExecBase *, SysBase, 1, Exec)
{
    AROS_LIBFUNC_INIT

    /* I have one more opener. */
    SysBase->LibNode.lib_OpenCnt++;
    return SysBase;
    AROS_LIBFUNC_EXIT
}

AROS_PLH0(BPTR, close,
    struct ExecBase *, SysBase, 2, Exec)
{
    AROS_LIBFUNC_INIT

    /* I have one fewer opener. */
    SysBase->LibNode.lib_OpenCnt--;
    return 0;
    AROS_LIBFUNC_EXIT
}
