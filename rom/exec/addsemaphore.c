/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$

    Desc: Add a semaphore to the public list of semaphores.
    Lang: english
*/
#include "exec_intern.h"
#include <exec/semaphores.h>
#include <proto/exec.h>

/*****************************************************************************

    NAME */

	AROS_LH1(void, AddSemaphore,

/*  SYNOPSIS */
	AROS_LHA(struct SignalSemaphore *, sigSem, A1),

/*  LOCATION */
	struct ExecBase *, SysBase, 100, Exec)

/*  FUNCTION
	Adds a semaphore to the system public semaphore list. Since the
	semaphore gets initialized by this function it must be free at
	this time. Also the ln_Name field must be set.

    INPUTS
	sigSem - Pointer to semaphore structure

    RESULT

    NOTES
	Semaphores are shared between the tasks that use them and must
	therefore lie in public (or at least shared) memory.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    /* Intialize semaphore */
#if 0 /* already done in InitSemaphore() */
    sigSem->ss_Link.ln_Type=NT_SIGNALSEM;
#endif
    InitSemaphore(sigSem);

    /* Arbitrate for the semaphore list */
    Forbid();
    /* Add the semaphore */
    Enqueue(&SysBase->SemaphoreList,&sigSem->ss_Link);

    /* All done. */
    Permit();
    AROS_LIBFUNC_EXIT
} /* AddSemaphore */

