/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Emit one character via raw IO
    Lang: english
*/

/*****i***********************************************************************

    NAME */
#include <aros/libcall.h>
#include <proto/exec.h>
#include <proto/kernel.h>

#include <stdarg.h>

extern void *priv_KernelBase;

static inline void bug(const char *format, ...)
{
    void *KernelBase = priv_KernelBase;
    va_list args;
    va_start(args, format);
    KrnBug(format, args);
    va_end(args);
}

	AROS_LH1(void, RawPutChar,

/*  SYNOPSIS */
        AROS_LHA(UBYTE, chr, D0),

/*  LOCATION */
        struct ExecBase *, SysBase, 86, Exec)

/*  FUNCTION
        Emits a single character.

    INPUTS
        chr - The character to emit

    RESULT
        None.

    NOTES
        This function is for very low level debugging only.

    EXAMPLE

    BUGS

    SEE ALSO
        RawIOInit(), RawPutChar(), RawMayGetChar()

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    /* Don't write 0 bytes */
    if (chr)
    {
        bug("%c",chr);
    }

    AROS_LIBFUNC_EXIT
} /* RawPutChar */
