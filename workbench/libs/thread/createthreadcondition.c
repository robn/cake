/*
 * thread.library - threading and synchronisation primitives
 *
 * Copyright � 2007 Robert Norris
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 */

#include "thread_intern.h"

#include <exec/memory.h>
#include <exec/lists.h>
#include <proto/exec.h>

/*****************************************************************************

    NAME */
        AROS_LH0(_ThreadCondition, CreateThreadCondition,

/*  SYNOPSIS */

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 14, Thread)

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

    _ThreadCondition cond;

    /* allocate */
    if ((cond = AllocMem(sizeof(struct _ThreadCondition), MEMF_PUBLIC)) == NULL)
        return NULL;

    /* initialise */
    InitSemaphore(&cond->lock);
    NEWLIST(&cond->waiters);
    cond->count = 0;

    return cond;

    AROS_LIBFUNC_EXIT
} /* CreateThreadCondition */