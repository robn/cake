/*
    Copyright � 2002-2003, The AROS Development Team. All rights reserved.
    $Id$
*/

#include "muimaster_intern.h"

/*****************************************************************************

    NAME */
	AROS_LH1(LONG, MUI_SetError,

/*  SYNOPSIS */
	AROS_LHA(LONG, num, D0),

/*  LOCATION */
	struct Library *, MUIMasterBase, 12, MUIMaster)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS
	The function itself is a bug ;-) Remove it!

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct MUIMasterBase *,MUIMasterBase)

    return 0;

    AROS_LIBFUNC_EXIT

} /* MUIA_SetError */
