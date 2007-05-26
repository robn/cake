/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Open a file with the specified mode.
    Lang: english
*/
#include <exec/memory.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <utility/tagitem.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include <dos/stdio.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH2(BPTR, Open,

/*  SYNOPSIS */
	AROS_LHA(CONST_STRPTR, name,       D1),
	AROS_LHA(LONG,         accessMode, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 5, Dos)

/*  FUNCTION
	Opens a file for read and/or write depending on the accessmode given.

    INPUTS
	name	   - NUL terminated name of the file.
	accessMode - One of MODE_OLDFILE   - open existing file
			    MODE_NEWFILE   - delete old, create new file
					     exclusive lock
			    MODE_READWRITE - open new one if it doesn't exist

    RESULT
	Handle to the file or 0 if the file couldn't be opened.
	IoErr() gives additional information in that case.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct FileHandle *fh;
    BPTR con, ast;
    LONG error;
    struct Process *me;

    /* Sanity check */
    if (name == NULL) return NULL;
    
    /* Get pointer to process structure */
    me = (struct Process *)FindTask(NULL);

    /* Create filehandle */
    fh = (struct FileHandle *)AllocDosObject(DOS_FILEHANDLE,NULL);

    if(fh != NULL)
    {
        LONG doappend = 0;

	/* Get pointer to I/O request. Use stackspace for now. */
	struct IOFileSys iofs;

	/* Prepare I/O request. */
	InitIOFS(&iofs, FSA_OPEN_FILE, DOSBase);

	/* io_Args[0] is the name which is set by DoName(). */
	switch(accessMode)
	{
	case MODE_OLDFILE:
	    iofs.io_Union.io_OPEN_FILE.io_FileMode = FMF_MODE_OLDFILE;
	    ast = con = me->pr_CIS;
	    break;

	case MODE_NEWFILE:
	    iofs.io_Union.io_OPEN_FILE.io_FileMode = FMF_MODE_NEWFILE;
	    con = me->pr_COS;
	    ast = me->pr_CES ? me->pr_CES : me->pr_COS;
	    break;

	case MODE_READWRITE:
	    iofs.io_Union.io_OPEN_FILE.io_FileMode = FMF_MODE_READWRITE;
	    con = me->pr_COS;
	    ast = me->pr_CES ? me->pr_CES : me->pr_COS;
	    break;

	default:
	    /* See if the user requested append mode */
	    doappend   = accessMode & FMF_APPEND;

	    /* The append mode is all taken care by dos.library */
	    accessMode &= ~FMF_APPEND;

	    iofs.io_Union.io_OPEN_FILE.io_FileMode = accessMode;
	    ast = con = me->pr_CIS;
	    break;
	}

	iofs.io_Union.io_OPEN_FILE.io_Protection = 0;

	if(!Stricmp(name, "IN:") || !Stricmp(name, "STDIN:"))
	{
	    iofs.IOFS.io_Device = FH_DEVICE(BADDR(me->pr_CIS));
	    iofs.IOFS.io_Unit   = FH_UNIT(BADDR(me->pr_CIS));
	    iofs.io_Union.io_OPEN_FILE.io_Filename = "";
	    DosDoIO(&iofs.IOFS);
	    error = me->pr_Result2 = iofs.io_DosError;
	}
	else
	if(!Stricmp(name, "OUT:") || !Stricmp(name, "STDOUT:"))
	{
	    iofs.IOFS.io_Device = FH_DEVICE(BADDR(me->pr_COS));
	    iofs.IOFS.io_Unit   = FH_UNIT(BADDR(me->pr_COS));
	    iofs.io_Union.io_OPEN_FILE.io_Filename = "";
	    DosDoIO(&iofs.IOFS);
	    error = me->pr_Result2 = iofs.io_DosError;
	}
	else
	if(!Stricmp(name, "ERR:") || !Stricmp(name, "STDERR:"))
	{
	    iofs.IOFS.io_Device = FH_DEVICE(BADDR(me->pr_CES ? me->pr_CES : me->pr_COS));
	    iofs.IOFS.io_Unit   = FH_UNIT(BADDR(me->pr_CES ? me->pr_CES : me->pr_COS));
	    iofs.io_Union.io_OPEN_FILE.io_Filename = "";
	    DosDoIO(&iofs.IOFS);
	    error = me->pr_Result2 = iofs.io_DosError;
	}
	else
	if(!Stricmp(name, "CONSOLE:"))
	{
	    iofs.IOFS.io_Device = FH_DEVICE(BADDR(con));
	    iofs.IOFS.io_Unit   = FH_UNIT(BADDR(con));
	    iofs.io_Union.io_OPEN_FILE.io_Filename = "";
	    DosDoIO(&iofs.IOFS);
	    error = me->pr_Result2 = iofs.io_DosError;
	}
	else
	if(!Stricmp(name, "*"))
	{
	    iofs.IOFS.io_Device = FH_DEVICE(BADDR(ast));
	    iofs.IOFS.io_Unit   = FH_UNIT(BADDR(ast));
	    iofs.io_Union.io_OPEN_FILE.io_Filename = "";
	    DosDoIO(&iofs.IOFS);
	    error = me->pr_Result2 = iofs.io_DosError;
	}
	else
	    error = DoName(&iofs, name, DOSBase);

	if(error == 0)
	{
            fh->fh_Type = (struct MsgPort *) iofs.IOFS.io_Device;
            fh->fh_Arg1 = iofs.IOFS.io_Unit;

	    if (doappend)
	    {
		 /* See if the handler supports FSA_SEEK */
		 if (Seek(MKBADDR(fh), 0, OFFSET_END) != -1)
		 {
		     /* if so then set the proper flag in the FileHandle struct */
		     fh->fh_Flags |= FHF_APPEND;
		 }
	    }
	    if (IsInteractive(MKBADDR(fh)))
	        SetVBuf(MKBADDR(fh), NULL, BUF_LINE, -1);

	    return MKBADDR(fh);
	}

	FreeDosObject(DOS_FILEHANDLE,fh);
    }

    else
	SetIoErr(ERROR_NO_FREE_STORE);

    return 0;

    AROS_LIBFUNC_EXIT
} /* Open */
