/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$
    $Log$
    Revision 1.7  1998/10/20 16:44:44  hkiel
    Amiga Research OS

    Revision 1.6  1997/01/27 00:36:24  ldp
    Polish

    Revision 1.5  1996/12/09 13:53:33  aros
    Added empty templates for all missing functions

    Moved #include's into first column

    Revision 1.4  1996/10/24 15:50:32  aros
    Use the official AROS macros over the __AROS versions.

    Revision 1.3  1996/08/13 13:52:48  digulla
    Replaced <dos/dosextens.h> by "dos_intern.h" or added "dos_intern.h"
    Replaced AROS_LA by AROS_LHA

    Revision 1.2  1996/08/01 17:40:54  digulla
    Added standard header for all files

    Desc:
    Lang: english
*/
#include <proto/exec.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH1(struct DosList *, LockDosList,

/*  SYNOPSIS */
	AROS_LHA(ULONG, flags, D1),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 109, Dos)

/*  FUNCTION
	Waits until the desired dos lists are free then gets a lock on them.
	A handle is returned that can be used for FindDosEntry().
	Calls to this function nest, i.e. you must call UnLockDosList()
	as often as you called LockDosList(). Always lock all lists
	at once - do not try to get a lock on one of them then on another.

    INPUTS
	flags - what lists to lock

    RESULT
	Handle to the dos list. This is not a direct pointer
	to the first list element but to a pseudo element instead.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    dos_lib.fd and clib/dos_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct DosLibrary *,DOSBase)
    if(flags&LDF_WRITE)
	ObtainSemaphore(&DOSBase->dl_DosListLock);
    else
	ObtainSemaphoreShared(&DOSBase->dl_DosListLock);
    return (struct DosList *)&DOSBase->dl_DevInfo;
    AROS_LIBFUNC_EXIT
} /* LockDosList */
