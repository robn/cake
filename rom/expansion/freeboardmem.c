/*
    (C) 1997 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/
#include "expansion_intern.h"

/*****************************************************************************

    NAME */
#include <clib/expansion_protos.h>

	AROS_LH2(void, FreeBoardMem,

/*  SYNOPSIS */
	AROS_LHA(ULONG, startSlot, D0),
	AROS_LHA(ULONG, slotSpec, D1),

/*  LOCATION */
	struct ExpansionBase *, ExpansionBase, 13, Expansion)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	27-11-96    digulla automatically created from
			    expansion_lib.fd and clib/expansion_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct ExpansionBase *,ExpansionBase)
    extern void aros_print_not_implemented (char *);

#warning TODO: Write expansion/FreeBoardMem()
    aros_print_not_implemented ("FreeBoardMem");

    AROS_LIBFUNC_EXIT
} /* FreeBoardMem */
