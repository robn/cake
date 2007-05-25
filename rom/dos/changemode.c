/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Change the mode of a filehandle or -lock.
    Lang: English
*/
#include <proto/exec.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH3(BOOL, ChangeMode,

/*  SYNOPSIS */
	AROS_LHA(ULONG, type,    D1),
	AROS_LHA(BPTR,  object,  D2),
	AROS_LHA(ULONG, newmode, D3),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 75, Dos)

/*  FUNCTION
	Try to change the mode used by a lock or filehandle.

    INPUTS
	type    - CHANGE_FH or CHANGE_LOCK.
	object  - Filehandle or lock.
	newmode - New mode (see <dos/dos.h>).

    RESULT
	!= 0 if all went well, otherwise 0. IoErr() gives additional
	information in the latter case.

    NOTES
	Since filehandles and locks are identical under AROS the type
	argument is ignored.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct IOFileSys iofs;

    InitIOFS(&iofs, FSA_FILE_MODE, DOSBase);

    if (type == CHANGE_FH) {
        struct FileHandle *fh = (struct FileHandle *) BADDR(object);
        iofs.IOFS.io_Device = FH_DEVICE(fh);
        iofs.IOFS.io_Unit   = FH_UNIT(fh);
    }
    else {
        struct FileLock *fl = (struct FileLock *) BADDR(object);
        iofs.IOFS.io_Device = FL_DEVICE(fl);
        iofs.IOFS.io_Unit   = FL_UNIT(fl);
    }

    if (newmode == EXCLUSIVE_LOCK)
        iofs.io_Union.io_FILE_MODE.io_FileMode = FMF_LOCK;
    else
        iofs.io_Union.io_FILE_MODE.io_FileMode = 0;

    iofs.io_Union.io_FILE_MODE.io_Mask = FMF_LOCK;

    /* Send the request. */
    DosDoIO(&iofs.IOFS);

    /* Set error code and return */
    if (iofs.io_DosError != 0)
    {
        SetIoErr(iofs.io_DosError);
	return DOSFALSE;
    }
    
    return DOSTRUE;

    AROS_LIBFUNC_EXIT
} /* ChangeMode */
