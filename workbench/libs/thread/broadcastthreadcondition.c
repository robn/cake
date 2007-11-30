/*
 * thread.library - threading and synchronisation primitives
 *
 * Copyright � 2007 Robert Norris
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 */

#include "thread_intern.h"

#include <exec/tasks.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <assert.h>

/*****************************************************************************

    NAME */
        AROS_LH1(void, BroadcastThreadCondition,

/*  SYNOPSIS */
        AROS_LHA(_ThreadCondition, cond, A0),

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 18, Thread)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    _ThreadWaiter waiter;

    assert(cond != NULL);

    /* safely operation on the condition */
    ObtainSemaphore(&cond->lock);

    /* wake up all the waiters */
    while ((waiter = (_ThreadWaiter) REMHEAD(&cond->waiters)) != NULL) {
        Signal(waiter->task, SIGF_SINGLE);
        FreeMem(waiter, sizeof(struct _ThreadWaiter));
    }

    /* none left */
    cond->count = 0;

    ReleaseSemaphore(&cond->lock);

    AROS_LIBFUNC_EXIT
} /* BroadcastThreadCondition */