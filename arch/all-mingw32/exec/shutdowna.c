/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id: coldreboot.c 18441 2003-07-07 20:01:00Z hkiel $

    Desc: ShutdownA() - Shut down the operating system.
    Lang: english
*/

#include <aros/debug.h>
#include "../kernel/hostinterface.h"

extern struct HostInterface *HostIFace;

/*****************************************************************************

    NAME */
#include <proto/exec.h>

	AROS_LH1(ULONG, ShutdownA,

/*  SYNOPSIS */
	AROS_LHA(ULONG, action, D0),

/*  LOCATION */
	struct ExecBase *, SysBase, 173, Exec)

/*  FUNCTION
	This function will shut down the operating system.

    INPUTS
	action - what to do:
	 * SD_ACTION_POWEROFF   - power off the machine.
	 * SD_ACTION_COLDREBOOT - cold reboot the machine (not only AROS).

    RESULT
	This function does not return in case of success. Otherwise is returns
	zero.

    NOTES
	It can be quite harmful to call this function. It may be possible that
	you will lose data from other tasks not having saved, or disk buffers
	not being flushed. Plus you could annoy the (other) users.

    EXAMPLE

    BUGS

    SEE ALSO
	ColdReboot()

******************************************************************************/
{
    AROS_LIBFUNC_INIT

    /* WinAPI CreateProcess() call may silently abort if scheduler attempts task switching
       while it's running. There's no sense in this beyond this point, so we simply Disable() */
    Disable();
    HostIFace->_Shutdown(action);
    Enable();
    return 0;

    AROS_LIBFUNC_EXIT
}
