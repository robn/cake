/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$
*/

#define DEBUG 0
#include <aros/debug.h>
#include "iffparse_intern.h"

/*****************************************************************************

    NAME */
#include <proto/iffparse.h>

	AROS_LH6(LONG, EntryHandler,

/*  SYNOPSIS */
	AROS_LHA(struct IFFHandle *, iff, A0),
	AROS_LHA(LONG              , type, D0),
	AROS_LHA(LONG              , id, D1),
	AROS_LHA(LONG              , position, D2),
	AROS_LHA(struct Hook      *, handler, A1),
	AROS_LHA(APTR              , object, A2),

/*  LOCATION */
	struct Library *, IFFParseBase, 17, IFFParse)

/*  FUNCTION
	Installs an entry handler for a specific chunk type
	that wil be called whenever a chunk of that type is pushed on the contextstack
	via ParseIFF().


    INPUTS
	iff	    - pointer to an iffhandle struct.
	type	  - type code for the chunk to handle. (ex: "ILBM").
	id	  -  ID code for the chunk to handle. (ex: "CMAP")
	position  -  position of localcontextitem. See StoreLocalItem for
		    more info.
	handler    -  an initialised Hook structure for the handler function.
	object	  -  pointer to some kind of object that will be passed to
		    your handler function.

    RESULT
	error - 0 If successfull, IFFERR_#? elsewise.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	ExitHandler(), StoreLocalItem(), StoreItemInContext()



    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct LocalContextItem *lci;
    struct HandlerInfo *hi;

    D(bug ("EntryHandler (iff=%p, type=%c%c%c%c, id=%c%c%c%c, position=%d, handler=%p, object=%p)\n",
	iff,
	dmkid(type),
	dmkid(id),
	position, handler, object
    ));

    if (!(lci = AllocLocalItem(
		type,
		id,
		IFFLCI_ENTRYHANDLER,
		sizeof (struct HandlerInfo))))
    {
	ReturnInt ("EntryHandler",LONG,IFFERR_NOMEM);
    }

    /* Get pointer to the user data */
    hi = LocalItemData(lci);

    hi->hi_Hook  = handler;

    hi->hi_Object  = object;

    ReturnInt
    (
	"EntryHandler",
	LONG,
	StoreLocalItem
	(
	    iff,
	    lci,
	    position
	)
    );

    AROS_LIBFUNC_EXIT
} /* EntryHandler */
