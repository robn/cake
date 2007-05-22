/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Assign CLI command
    Lang: English
*/

#define  DEBUG  0
#include <aros/debug.h>
#include <aros/config.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <utility/tagitem.h>

#include <stdio.h>
#include <string.h>

#include <aros/shcommands.h>

/******************************************************************************


    NAME

        Assign [(name):] [{(target)}] [LIST] [EXISTS] [DISMOUNT] [DEFER]
	       [PATH] [ADD] [REMOVE] [VOLS] [DIRS] [DEVICES]

    SYNOPSIS

        NAME, TARGET/M, LIST/S, EXISTS/S, DISMOUNT/S, DEFER/S, PATH/S, ADD/S,
	REMOVE/S, VOLS/S, DIRS/S, DEVICES/S

    LOCATION

        Workbench:C

    FUNCTION

        ASSIGN creates a reference to a file or directory. The reference 
	is a logical device name which makes it very convenient to specify
	assigned objects using the reference instead of their paths.

	If the NAME and TARGET arguments are given, ASSIGN assigns the
	given logical name to the specified target. If the NAME given is
	already assigned to a file or directory the new target replaces the
	previous target. A colon must be included after the NAME argument.

	If only the NAME argument is given, any assigns to that NAME are
	removed. If no arguments whatsoever are given, all logical
	assigns are listed.

    INPUTS

        NAME      --  the name that should be assigned to a file or directory
	TARGET    --  one or more files or directories to assign the NAME to
	LIST      --  list all assigns made
	EXISTS    --  if NAME is already assigned, set the condition flag to
	              WARN
	DISMOUNT  --  remove the volume or device NAME from the dos list
	DEFER     --  make an ASSIGN to a path or directory that not need to
	              exist at the time of assignment. The first time the
		      NAME is referenced the NAME is bound to the object
	PATH      --  path to assign with a non-binding assign. This means
	              that the assign is re-evaluated each time a reference
		      to NAME is done. Like for DEFER, the path doesn't have
		      to exist when the ASSIGN command is executed
	ADD       --  don't replace an assign but add another object for a
                      NAME (multi-assigns)
	REMOVE    --  remove an ASSIGN
	VOLS      --  show assigned volumes if in LIST mode
	DIRS      --  show assigned directories if in LIST mode
	DEVICES   --  show assigned devices if in LIST mode
        

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

        The assign command has many switches. This together with the somewhat
	messy handling of DosList:s by dos.library makes the operation rather
	complicated.

	There are some fundamental building blocks that defines the semantics
	of the Assign command.

	Only one of the switches ADD, REMOVE, PATH and DEFER may be specified.

	If EXISTS is specified, only the name parameter is important.

	The implementation is split up in two fundamental procedures.

	doAssign()     --  make [a number of] assigns
	showAssigns()  --  show the available assigns
	checkAssign()  --  check if a particular assign exists

    HISTORY

******************************************************************************/
/* Prototypes */

static int checkAssign(struct ExecBase *SysBase, struct DosLibrary *DOSBase, STRPTR name);
static int doAssign(struct DosLibrary *DOSBase, STRPTR name, STRPTR *target,
		    BOOL dismount, BOOL defer, BOOL path,
		    BOOL add, BOOL remove);
static void showAssigns(struct ExecBase *SysBase, struct DosLibrary *DOSBase,
			BOOL vols, BOOL dirs, BOOL devices);
static int removeAssign(struct DosLibrary *DOSBase, STRPTR name);
static STRPTR GetFullPath(struct ExecBase *SysBase, struct DosLibrary *DOSBase,
			  BPTR lock);

AROS_SH12(Assign, 41.5,
AROS_SHA(STRPTR, ,NAME, ,NULL),
AROS_SHA(STRPTR *, ,TARGET,/M,NULL),
AROS_SHA(BOOL, ,LIST,/S,FALSE),
AROS_SHA(BOOL, ,EXISTS,/S,FALSE),
AROS_SHA(BOOL, ,DISMOUNT,/S,FALSE),
AROS_SHA(BOOL, ,DEFER,/S,FALSE),
AROS_SHA(BOOL, ,PATH,/S,FALSE),
AROS_SHA(BOOL, ,ADD,/S,FALSE),
AROS_SHA(BOOL, ,REMOVE,/S,FALSE),
AROS_SHA(BOOL, ,VOLS,/S,FALSE),
AROS_SHA(BOOL, ,DIRS,/S,FALSE),
AROS_SHA(BOOL, ,DEVICES,/S,FALSE))
{
    AROS_SHCOMMAND_INIT
    int error = RETURN_OK;

    if (SHArg(ADD) + SHArg(REMOVE) + SHArg(PATH) + SHArg(DEFER) > 1)
    {
	PutStr("Only one of ADD, REMOVE, PATH or DEFER is allowed\n");

	return RETURN_FAIL;
    }

    /* Check device name
     */
    if (SHArg(NAME))
    {
	char *pos;

	/* Correct assign name construction? The rule is that the device name
	 * should end with a colon at the same time as no other colon may be
	 * in the name.
	 */
	pos = strchr(SHArg(NAME), ':');
	if (!pos || pos[1])
	{
		VPrintf("Invalid device name %s\n", &SHArg(NAME));
		return RETURN_FAIL;
	}
    }
    /* If the EXISTS keyword is specified, we only care about NAME */
    if (SHArg(EXISTS))
    {
        error = checkAssign(SysBase, DOSBase, SHArg(NAME));
    }
    else if (SHArg(NAME) != NULL)
    {
	/* If a NAME is specified, our primary task is to add or
	   remove an assign */

        error = doAssign(DOSBase, SHArg(NAME), SHArg(TARGET), SHArg(DISMOUNT),
			 SHArg(DEFER), SHArg(PATH), SHArg(ADD), SHArg(REMOVE));

        if (SHArg(LIST))
	{
	    /* With the LIST keyword, the current assigns will be
	       displayed also when (after) making an assign */
	    
	    showAssigns(SysBase, DOSBase, SHArg(VOLS), SHArg(DIRS),
			SHArg(DEVICES));
	}
    }
    else
    {
	/* If no NAME was given, we just show the current assigns
	   as specified by the user (VOLS, DIRS, DEVICES) */

	showAssigns(SysBase, DOSBase, SHArg(VOLS), SHArg(DIRS),
		    SHArg(DEVICES));
    }

    return error;

    AROS_SHCOMMAND_EXIT
}


static void showAssigns(struct ExecBase *SysBase, struct DosLibrary *DOSBase,
			BOOL vols, BOOL dirs, BOOL devices)
{
    ULONG           lockBits = LDF_READ;
    struct DosList *dl;
    
    /* If none of the parameters are specified, everything should be
       displayed */
    if (!(vols || dirs || devices))
    {
	vols    = TRUE;
	dirs    = TRUE;
	devices = TRUE;
    }
    
    lockBits |= vols    ? LDF_VOLUMES : 0;
    lockBits |= dirs    ? LDF_ASSIGNS : 0;
    lockBits |= devices ? LDF_DEVICES : 0;
    
    dl = LockDosList(lockBits);
    
    if (vols)
    {
	struct DosList *tdl = dl;	/* Loop variable */
	
	PutStr("Volumes:\n");

	/* Print all mounted volumes */
	while ((tdl = NextDosEntry(tdl, LDF_VOLUMES)) != NULL)
	{
	    VPrintf("%s [Mounted]\n", (IPTR *)&tdl->dol_Ext.dol_AROS.dol_DevName);
	}
    }
    
    if (dirs)
    {
	struct DosList *tdl = dl;       /* Loop variable */
	int             count;	        /* Loop variable */
	
	PutStr("Directories:\n");
	
	/* Print all assigned directories */
	while ((tdl = NextDosEntry(tdl, LDF_ASSIGNS)) != NULL)
	{
	    PutStr(tdl->dol_Ext.dol_AROS.dol_DevName);
	    
	    for(count = 14 - strlen(tdl->dol_Ext.dol_AROS.dol_DevName); count > 0; count--)
	    {
		PutStr(" ");
	    }
	    
	    switch (tdl->dol_Type)
	    {
	    case DLT_LATE:
		VPrintf("<%s>\n",
			(IPTR *)&tdl->dol_misc.dol_assign.dol_AssignName);
		break;
		
	    case DLT_NONBINDING:
		VPrintf("[%s]\n", 
			(IPTR *)&tdl->dol_misc.dol_assign.dol_AssignName);
		break;
		
	    default:
		{
		    STRPTR             dirName;     /* For NameFromLock() */
		    struct AssignList *nextAssign;  /* For multiassigns */
		    
		    dirName = GetFullPath(SysBase, DOSBase, tdl->dol_Lock);
		    
		    if (dirName != NULL)
		    {
			VPrintf("%s\n", (IPTR *)&dirName);
			FreeVec(dirName);
		    }
		    else
		    {
			PutStr("\n");
		    }

		    nextAssign = tdl->dol_misc.dol_assign.dol_List;
		    
		    while (nextAssign != NULL)
		    {
			dirName = GetFullPath(SysBase, DOSBase,
					      nextAssign->al_Lock);
			
			if (dirName != NULL)
			{
			    VPrintf("             + %s\n", (IPTR *)&dirName);
			    FreeVec(dirName);
			}
			
			nextAssign = nextAssign->al_Next;
		    }
		}
		
		break;
	    }
	} /* while(NextDosEntry() != NULL) */
    }
    
    if (devices)
    {
	struct DosList *tdl = dl;     /* Loop variable */
	int             count = 0;    /* Used to make sure that as most 5
				         entries are printed per line */
	
	PutStr("Devices:\n");
	
	/* Print all assigned devices */
	while ((tdl = NextDosEntry(tdl, LDF_DEVICES)) != NULL)
	{
	    VPrintf("%s ", (IPTR *)&tdl->dol_Ext.dol_AROS.dol_DevName);
	    count++;
	    
	    if (count == 5)
	    {
		PutStr("\n");
		count = 0;
	    }
	}

	if (count <  5)
	{
	    PutStr("\n");
	}
    }
    
    UnLockDosList(lockBits);
}


static STRPTR GetFullPath(struct ExecBase *SysBase, struct DosLibrary *DOSBase,
			  BPTR lock)
{
    UBYTE  *buf;       /* Pointer to the memory allocated for the string */
    ULONG   size;      /* Holder of the (growing) size of the string */
    
    for (size = 512; ; size += 512)
    {
	buf = AllocVec(size, MEMF_ANY);
	
	if (buf == NULL)
	{
	    break;
	}
	
	if (NameFromLock(lock, buf, size))
	{
	    return (STRPTR)buf;
	}
	
	FreeVec(buf);

        if (IoErr() != ERROR_LINE_TOO_LONG)
        {
            break;
        }
    }
    
    return NULL;
}


static int doAssign(struct DosLibrary *DOSBase, STRPTR name, STRPTR *target,
		    BOOL dismount, BOOL defer, BOOL path, BOOL add,
		    BOOL remove)
{
    STRPTR colon;
    BPTR   lock = NULL;
    int    i;			/* Loop variable */

    int  error = RETURN_OK;
    LONG ioerr = 0;
    BOOL cancel = FALSE;
    BOOL success = TRUE;
   
    colon = strchr(name, ':');
    *colon = 0;		      /* Remove trailing colon; name[] is changed! */

    /* This is a little bit messy... We first remove the 'name' assign
       and later in the loop the target assigns. */
    if (dismount)
    {

	struct DosList *dl;
	struct DosList *fdl;

	dl = LockDosList(LDF_VOLUMES | LDF_DEVICES | LDF_WRITE);

	fdl = FindDosEntry(dl, name, LDF_VOLUMES | LDF_DEVICES);

	/* Note the ! for conversion to boolean value */
	if (fdl)
		success = RemDosEntry(fdl);

	UnLockDosList(LDF_VOLUMES | LDF_DEVICES | LDF_WRITE);
    }
    else
    {
        if(target == NULL || *target == NULL || remove)
        {
		error = removeAssign(DOSBase, name);
		if (error)
		{
			ioerr = IoErr();
			cancel = TRUE;
		}
        }
    }
    /* AmigaDOS doesn't use RETURN_WARN here... but it should? */
    error = success ? error : RETURN_WARN;

    // The Loop over multiple targets starts here

    if (target)
    {
	for (i = 0; target[i] != NULL; i++)
        {
		cancel = FALSE;
		if (!(path || defer || dismount))
		{
	    		lock = Lock(target[i], SHARED_LOCK);
	    
	    		if (lock == NULL)
	    		{
				VPrintf("Can't find %s\n", (IPTR *)&target[i]);

				return RETURN_FAIL;
	    		}
		}

		if (remove)
		{
	    		AssignLock(target[i], NULL);
	    		UnLock(lock);
		}
		else if (path)
		{
			if (!AssignPath(name, target[i]))
			{
				ioerr = IoErr();
				error = RETURN_FAIL;
			}
		}
		else if (add)
		{
	    		if (AssignAdd(name, lock) == DOSFALSE)
	    		{
				struct DosList *dl;

				error = RETURN_FAIL;
				ioerr = IoErr();
				/* Check if the assign doesn't exist at all. If so, create it.
				 * This fix bug id 145. - Piru
				 */
				dl = LockDosList(LDF_ASSIGNS | LDF_READ);
				dl = FindDosEntry(dl, name, LDF_ASSIGNS);
				UnLockDosList(LDF_ASSIGNS | LDF_READ);

				if (!dl)
				{
					if (AssignLock(name, lock))
					{
						error = RETURN_OK;
						lock = NULL;
					}
					else
					{
						ioerr = IoErr();
					}
				}

				if (lock)
					UnLock(lock);

				if (error && ioerr != ERROR_OBJECT_EXISTS)
				{
					STRPTR Args[] = {target[i], name};
					VPrintf("Can't add %s to %s\n", Args);
				}
			}
		}
		else if (defer)
		{
	    		if (AssignLate(name, target[i]) == DOSFALSE)
	    		{
				ioerr = IoErr();
				UnLock(lock);
				error = RETURN_FAIL;
		    	}
		}
		else
		{
	    		/* If no extra parameters are specified we just do a regular
	    		   assign (replacing any possible previous assign with that
	    		   name. The case of target being NULL is taken care of above. */
	    		if (AssignLock(name, lock) == DOSFALSE)
	    		{
				ioerr = IoErr();
				cancel = TRUE;
				UnLock(lock);
				error = RETURN_FAIL;
	    		}

            		/* If there are several targets, the next ones have to be added. */
            		add = TRUE;
		}
	
		/* We break as soon as we get a serious error */
		if (error >= RETURN_FAIL)
		{
			return error;
		}

	} /* loop through all targets */
    }
    if (error)
    {
	if (ioerr == ERROR_OBJECT_EXISTS)
	{
		STRPTR Args[] = {cancel ? "cancel" : "assign", name};
		VPrintf("Can't %s %s\n", Args);
	}
	else
	{
		PrintFault(ioerr, NULL);
	}
    }
    return error;
}

static int removeAssign(struct DosLibrary *DOSBase, STRPTR name)
{
	/* In case no target is given, the 'name' assign should be removed.
	 * The AmigaDOS semantics for this is apparently that the error
	 * code is never set even if the assign didn't exist.
	 */

	if (!AssignLock(name, NULL))
	{
		return RETURN_FAIL;
	}
	return RETURN_OK;
}

static int checkAssign(struct ExecBase *SysBase, struct DosLibrary *DOSBase, STRPTR name)
{
    STRPTR colon;
    struct DosList *dl;
    int             error = RETURN_OK;

    if (!name)
	name = "";

    colon = strchr(name, ':');
    if (colon)
    {
	*colon = '\0';
    }

    dl = LockDosList(LDF_DEVICES | LDF_ASSIGNS | LDF_VOLUMES | LDF_READ);
    dl = FindDosEntry(dl, name, LDF_DEVICES | LDF_ASSIGNS | LDF_VOLUMES);
    if (dl)
    {
	struct DosList *tdl = dl;
	int             count;

	switch (dl->dol_Type)
	{
	case DLT_DEVICE:
		VPrintf("%s\n", &tdl->dol_Name);
		break;

	case DLT_VOLUME:
		VPrintf("%s [Mounted]\n", &tdl->dol_Name);
		break;

	case DLT_DIRECTORY:
	case DLT_LATE:
	case DLT_NONBINDING:

		VPrintf("%s ", &tdl->dol_Name);

		for (count = 14 - *((UBYTE*)tdl->dol_Name); count > 0; count--)
		{
			PutStr(" ");
		}

		switch (tdl->dol_Type)
		{
		case DLT_LATE:
			VPrintf("<%s>\n", (IPTR) &tdl->dol_misc.dol_assign.dol_AssignName);
			break;

		case DLT_NONBINDING:
			VPrintf("[%s]\n", (IPTR) &tdl->dol_misc.dol_assign.dol_AssignName);
			break;

		default:
			{
				STRPTR             dirName;     /* For NameFromLock() */
				struct AssignList *nextAssign;  /* For multiassigns */

				dirName = GetFullPath(SysBase, DOSBase, tdl->dol_Lock);

				if (dirName)
				{
					PutStr(dirName);
					FreeVec(dirName);
				}
				PutStr("\n");

				nextAssign = tdl->dol_misc.dol_assign.dol_List;

				while (nextAssign)
				{
					dirName = GetFullPath(SysBase, DOSBase, nextAssign->al_Lock);

					if (dirName)
					{
						VPrintf("             + %s\n", (IPTR) &dirName);
						FreeVec(dirName);
					}

					nextAssign = nextAssign->al_Next;
				}
			}
		}
		break;
	}
    }
    else
    {
	VPrintf("%s: not assigned\n", &name);

	error = RETURN_WARN;
    }
    UnLockDosList(LDF_DEVICES | LDF_ASSIGNS | LDF_VOLUMES | LDF_READ);
    if (colon)
	*colon = ':';
    return error;
}
