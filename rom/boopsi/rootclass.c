/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: BOOPSI rootclass
    Lang: english
*/

#include <exec/lists.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/alib.h>
#include <proto/arossupport.h>
#ifndef INTUITION_CLASSES_H
#   include <intuition/classes.h>
#endif
#ifndef UTILITY_HOOKS_H
#   include <utility/hooks.h>
#endif
#include <utility/utility.h>
#include <aros/asmcall.h>
#include "intern.h"

AROS_UFP3S(ULONG, rootDispatcher,
    AROS_UFPA(Class *,  cl,  A0),
    AROS_UFPA(Object *, obj, A2),
    AROS_UFPA(Msg,      msg, A1)
);

Class rootclass =
{
    { { NULL, NULL }, rootDispatcher, NULL, NULL },
    0,		/* reserved */
    NULL,	/* No superclass */
    (ClassID)ROOTCLASS,  /* ClassID */

    0, 0,	/* No offset and size */

    0,		/* UserData */
    0,		/* SubClassCount */
    0,		/* ObjectCount */
    0,		/* Flags */
};


#define BOOPSIBase	(GetBBase(cl->cl_UserData))

/*****i************************************************************************

    NAME */
	AROS_UFH3S(IPTR, rootDispatcher,

/*  SYNOPSIS */
	AROS_UFHA(Class  *, cl,  A0),
	AROS_UFHA(Object *, o,   A2),
	AROS_UFHA(Msg,      msg, A1))

/*  FUNCTION
	internal !

	Processes all messages sent to the RootClass. Unknown messages are
	silently ignored.

    INPUTS
	cl - Pointer to the RootClass
	o - This object was the destination for the message in the first
		place
	msg - This is the message.

    RESULT
	Processes the message. The meaning of the result depends on the
	type of the message.

    NOTES
	This is a good place to debug BOOPSI objects since every message
	should eventually show up here.

    EXAMPLE

    BUGS

    SEE ALSO

    HISTORY
	14.09.93    ada created

******************************************************************************/
{
    AROS_USERFUNC_INIT
    IPTR retval = 0;
    Class *objcl;

    switch (msg->MethodID)
    {
    case OM_NEW: {
	objcl = (Class *)o;

	/* Get memory. The objects shows how much is needed.
	   (The object is not an object, it is a class pointer!) */
	o = (Object *) AllocVec (objcl->cl_InstOffset
		+ objcl->cl_InstSize
		+ sizeof (struct _Object)
	    , MEMF_ANY|MEMF_CLEAR
	    );

	((struct _Object *)o)->o_Class = objcl;
	retval = (IPTR) BASEOBJECT(o);
	break; }

    case OM_DISPOSE:
	/* Free memory. Caller is responsible that everything else
	   is already cleared! */
	FreeVec (_OBJECT(o));
	break;

    case OM_ADDTAIL:
	/* Add <o> to list. */
	AddTail (((struct opAddTail *)msg)->opat_List,
		    (struct Node *) _OBJECT(o));
	break;

    case OM_REMOVE:
	/* Remove object from list. */
	Remove ((struct Node *) _OBJECT(o));
	break;

    case OM_SET:
    case OM_GET:
    case OM_UPDATE:
    case OM_NOTIFY:
    case OM_ADDMEMBER:
    case OM_REMMEMBER:
    default:
	/* Ignore */
	break;

    } /* switch */

    return (retval);
    AROS_USERFUNC_EXIT
} /* rootDispatcher */
