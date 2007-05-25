/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Locks a file or directory.
    Lang: English
*/

#include <proto/exec.h>
#include <utility/tagitem.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include "dos_intern.h"

#define  DEBUG  0
#include <aros/debug.h>


/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH2(BPTR, Lock,

/*  SYNOPSIS */
	AROS_LHA(CONST_STRPTR, name,       D1),
	AROS_LHA(LONG,         accessMode, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 14, Dos)

/*  FUNCTION
	Gets a lock on a file or directory. There may be more than one
	shared lock on a file but only one if it is an exclusive one.
	Locked files or directories may not be deleted.

    INPUTS
	name	   - NUL terminated name of the file or directory.
	accessMode - One of SHARED_LOCK
			    EXCLUSIVE_LOCK

    RESULT
	Handle to the file or directory or 0 if the object couldn't be locked.
	IoErr() gives additional information in that case.

    NOTES
	The lock structure returned by this function is different
	from that of AmigaOS (in fact it is identical to a filehandle).
	Do not try to read any internal fields.

*****************************************************************************/

{
    AROS_LIBFUNC_INIT

    struct Process *me;
    struct FileLock *fl;
    struct IOFileSys iofs;
    BPTR con = NULL, ast = NULL;
    LONG error;

    ASSERT_VALID_PTR(name);

    /* Sanity check */
    if (name == NULL)
    {
        return NULL;
    }
    
    /* Get pointer to process structure */
    me = (struct Process *)FindTask(NULL);

    /* Create lock */
    fl = (struct FileLock *) AllocMem(sizeof(struct FileLock), MEMF_CLEAR);
    if (fl == NULL) {
        SetIoErr(ERROR_NO_FREE_STORE);
        return NULL;
    }

    /* Prepare I/O request. */
    InitIOFS(&iofs, FSA_OPEN, DOSBase);
    
    /* io_Args[0] is the name which is set by DoName(). */
    switch (accessMode)
    {
        case EXCLUSIVE_LOCK:
            iofs.io_Union.io_OPEN.io_FileMode = FMF_LOCK | FMF_READ;
            con = me->pr_COS;
            ast = me->pr_CES ? me->pr_CES : me->pr_COS;
            break;
        
        case SHARED_LOCK:
            iofs.io_Union.io_OPEN.io_FileMode = FMF_READ;
            con = ast = me->pr_CIS;
            break;
    
        default:
            iofs.io_Union.io_OPEN.io_FileMode = accessMode;
            con = ast = me->pr_CIS;
            break;
    }
    
#define iofs_SetDeviceUnit(_iofs, _fl) \
{ \
     (_iofs).IOFS.io_Device = ((struct FileLock *)BADDR((_fl)))->fl_Device; \
     (_iofs).IOFS.io_Unit   = ((struct FileLock *)BADDR((_fl)))->fl_Unit; \
}
    
    if(!Stricmp(name, "IN:") || !Stricmp(name, "STDIN:"))
    {
        iofs_SetDeviceUnit(iofs, me->pr_CIS);

        iofs.io_Union.io_OPEN_FILE.io_Filename = "";
    
        DosDoIO(&iofs.IOFS);
    
        error = me->pr_Result2 = iofs.io_DosError;
    }
    else if(!Stricmp(name, "OUT:") || !Stricmp(name, "STDOUT:"))
    {
        iofs_SetDeviceUnit(iofs, me->pr_COS);
    
        iofs.io_Union.io_OPEN_FILE.io_Filename = "";
    
        DosDoIO(&iofs.IOFS);
    
        error = me->pr_Result2 = iofs.io_DosError;
    }
    else if(!Stricmp(name, "ERR:") || !Stricmp(name, "STDERR:"))
    {
        if (NULL != me->pr_CES)
        {
            iofs_SetDeviceUnit(iofs, me->pr_CES);
        }
        else
        {
            iofs_SetDeviceUnit(iofs, me->pr_COS);
        }
    
        iofs.io_Union.io_OPEN_FILE.io_Filename = "";
    
        DosDoIO(&iofs.IOFS);
    
        error = me->pr_Result2 = iofs.io_DosError;
    }
    else if(!Stricmp(name, "CONSOLE:"))
    {
        iofs_SetDeviceUnit(iofs, con);
    
        iofs.io_Union.io_OPEN_FILE.io_Filename = "";
    
        DosDoIO(&iofs.IOFS);
    
        error = me->pr_Result2 = iofs.io_DosError;
        }
    else if(!Stricmp(name, "*"))
    {
        iofs_SetDeviceUnit(iofs, ast);

        iofs.io_Union.io_OPEN_FILE.io_Filename = "";
    
        DosDoIO(&iofs.IOFS);
    
        error = me->pr_Result2 = iofs.io_DosError;
    }
    else
    {
        error = DoName(&iofs, name, DOSBase);
    }

    if (error != 0) {
        FreeMem(fl, sizeof(struct FileLock));
        return NULL;
    }
    
    FL_DEVICE(fl) = iofs.IOFS.io_Device;
    FL_UNIT(fl)   = iofs.IOFS.io_Unit;
    
    return MKBADDR(fl);

    AROS_LIBFUNC_EXIT
} /* Lock */
