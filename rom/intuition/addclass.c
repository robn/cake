/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.1  1996/08/28 17:55:34  digulla
    Proportional gadgets
    BOOPSI


    Desc:
    Lang: english
*/
#include <clib/exec_protos.h>
#include "intuition_intern.h"

/*****************************************************************************

    NAME */
	#include <intuition/classes.h>
	#include <clib/intuition_protos.h>

	__AROS_LH1(void, AddClass,

/*  SYNOPSIS */
	__AROS_LHA(struct IClass *, classPtr, A0),

/*  LOCATION */
	struct IntuitionBase *, IntuitionBase, 114, Intuition)

/*  FUNCTION
	Makes a class publically usable. This function must not be called
	before MakeClass().

    INPUTS
	class - The result of MakeClass()

    RESULT
	None.

    NOTES

    EXAMPLE
	There is no protection against creating multiple classes with
	the same name yet. The operation of the system is undefined
	in this case.

    BUGS

    SEE ALSO
	MakeClass(), FreeClass(), RemoveClass(), "Basic Object-Oriented
	Programming System for Intuition" and "boopsi Class Reference"

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    intuition_lib.fd and clib/intuition_protos.h

*****************************************************************************/
{
    __AROS_FUNC_INIT
    __AROS_BASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)

    AddTail (PublicClassList, (struct Node *)classPtr);

    classPtr->cl_Flags |= CLF_INLIST;

    __AROS_FUNC_EXIT
} /* AddClass */
