/*
    Copyright � 1995-2008, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Execute a loaded command synchronously
    Lang: english
*/
#include <exec/memory.h>
#include "../exec/etask.h"
#include <proto/exec.h>
#include <utility/tagitem.h>
#include <dos/filesystem.h>
#include <proto/dos.h>
#include "dos_intern.h"

LONG AROS_SLIB_ENTRY(RunProcess,Dos)
(
	struct Process	       * proc,
	struct StackSwapStruct * sss,
	CONST_STRPTR		 argptr,
	ULONG			 argsize,
	LONG_FUNC		 entry,
	struct DosLibrary      * DOSBase
);

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH4(LONG, RunCommand,

/*  SYNOPSIS */
	AROS_LHA(BPTR,   segList,   D1),
	AROS_LHA(ULONG,  stacksize, D2),
	AROS_LHA(CONST_STRPTR, argptr,    D3),
	AROS_LHA(ULONG,  argsize,   D4),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 84, Dos)

/*  FUNCTION
	RunCommand() will run the command loaded in the |segList| with the
	arguments specified with a new stack of |stacksize| bytes. Note
	that the stacksize may be extended if this is required.

	The return code of the command run will be returned.

	AROS ONLY: RunCommand() automatically computes argsize
	automatically if you pass -1.

	This call will not return until the command has completed.

    INPUTS
	segList		- segment of program to run.
	stacksize	- size of the stack to use.
	argptr		- pointer to NULL-terminated arguments.
	argsize		- size of the arguments string.

    RESULT
	The return code from the program. See also IoErr().

    NOTES
	Programs expect the argument string to end with a newline ('\n')
	character (ReadArgs() requires it to work properly).

    EXAMPLE

    BUGS

    SEE ALSO
	SystemTagList()

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    STRPTR oldargs;
    LONG oldresult;
    struct aros_startup * oldstartup;

    /* Get pointer to process structure */
    struct Process *me=(struct Process *)FindTask(NULL);

    UBYTE *stack;
    LONG ret;
    struct StackSwapStruct sss;

    if(stacksize < AROS_STACKSIZE)
	stacksize = AROS_STACKSIZE;

    stack=(UBYTE *)AllocMem(stacksize,MEMF_ANY);
    if(stack==NULL)
	return -1;

    sss.stk_Lower=stack;
    sss.stk_Upper=stack+stacksize;

    oldresult=me->pr_Result2;
    /* we have to save iet_startup field because it's overwritten in 
       startup code */
	oldstartup = (struct aros_startup *)GetIntETask(me)->iet_startup;
    
    me->pr_Result2=oldresult;

    oldargs=me->pr_Arguments;
    me->pr_Arguments=(STRPTR)argptr;

    ret=AROS_SLIB_ENTRY(RunProcess,Dos)(me,&sss,argptr,argsize,
		(LONG_FUNC)((BPTR *)BADDR(segList)+1),DOSBase);
    me->pr_Arguments=oldargs;

    oldresult=me->pr_Result2;
    /* restore saved iet_startup */
    GetIntETask(me)->iet_startup = oldstartup;

    me->pr_Result2=oldresult;

    FreeMem(stack,stacksize);
    
    return ret;
    
    AROS_LIBFUNC_EXIT
    
} /* RunCommand */
