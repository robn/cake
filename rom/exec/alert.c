/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.2  1996/08/01 17:27:03  digulla
    Added copyright notics and made headers conform

    Desc:
    Lang:
*/
#include <exec/execbase.h>
#include <aros/libcall.h>
#include <stdio.h>
#include <stdlib.h>

/*****************************************************************************

    NAME */
	#include <exec/alerts.h>
	#include <clib/exec_protos.h>

	__AROS_LH1(void, Alert,

/*  SYNOPSIS */
	__AROS_LA(unsigned long, alertNum, D7),

/*  LOCATION */
	struct ExecBase *, SysBase, 18, Exec)

/*  FUNCTION
	Alerts the user of a serious system problem.

    INPUTS
	alertNum - This is a number which contains information about
		the reason for the call.

    RESULT
	This routine may return, if the alert is not a dead-end one.

    NOTES
	You should not call this routine because it halts the machine,
	displays the message and then may reboot it.

    EXAMPLE
	// Dead-End alert: 680x0 Access To Odd Address
	Alert (0x80000003);

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	26-08-95    digulla created after EXEC-Routine

******************************************************************************/
{
    __AROS_FUNC_INIT

    /* since this is an emulation, we just show the bug in the console */
    fprintf (stderr
	, "GURU Meditation %04lx %04lx %s\nTask: %p\n"
	, alertNum >> 16
	, alertNum & 0xFFFF
	, (alertNum & 0x80000000) ? "(DEADEND)" : ""
	, FindTask (NULL)
	);

    if (alertNum & AT_DeadEnd)
	exit (20);

    __AROS_FUNC_EXIT
} /* Alert */

