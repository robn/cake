/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/
#include "expansion_intern.h"

/*****************************************************************************

    NAME */
#include <clib/expansion_protos.h>

	AROS_LH1(void, AllocBoardMem,

/*  SYNOPSIS */
	AROS_LHA(ULONG, slotSpec, D0),

/*  LOCATION */
	struct ExpansionBase *, ExpansionBase, 7, Expansion)

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

#warning TODO: Write expansion/AllocBoardMem()
    aros_print_not_implemented ("AllocBoardMem");

    AROS_LIBFUNC_EXIT
} /* AllocBoardMem */
