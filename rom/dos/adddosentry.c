/*
    $Id$
    $Log$
    Revision 1.1  1996/07/28 16:37:22  digulla
    Initial revision

    Desc:
    Lang: english
*/
#include <dos/dosextens.h>
#include <clib/utility_protos.h>

/*****************************************************************************

    NAME */
	#include <clib/dos_protos.h>

	__AROS_LH1(LONG, AddDosEntry,

/*  SYNOPSIS */
	__AROS_LA(struct DosList *, dlist, D1),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 113, Dos)

/*  FUNCTION
	Adds a given dos list entry to the dos list. Automatically
	locks the list for writing. There may be not more than one device
	or assign node of the same name. There are no restrictions on
	volume nodes.

    INPUTS
	dlist - pointer to dos list entry.

    RESULT
	!=0 if all went well, 0 otherwise.

    NOTES
	Since anybody who wants to use a device or volume node in the
	dos list has to lock the list, filesystems may be called with
	the dos list locked. So if you want to add a dos list entry
	out of a filesystem don't just wait on the lock but serve all
	incoming requests until the dos list is free instead.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    dos_lib.fd and clib/dos_protos.h

*****************************************************************************/
{
    __AROS_FUNC_INIT
    __AROS_BASE_EXT_DECL(struct DosLibrary *,DOSBase)
    LONG success=1;
    struct DosList *dl;

    dl=LockDosList(LDF_ALL|LDF_WRITE);
    if(dlist->dol_Type!=DLT_VOLUME)
    {
	dl=FindDosEntry(dl,dlist->dol_Name,LDF_DEVICES|LDF_ASSIGNS|LDF_WRITE);
	if(dl!=NULL)
	    success=0;
    }
    if(success)
    {
	dlist->dol_Next=DOSBase->dl_DevInfo;
	DOSBase->dl_DevInfo=dlist;
    }
    UnLockDosList(LDF_ALL|LDF_WRITE);

    return success;    
    __AROS_FUNC_EXIT
} /* AddDosEntry */
