/*
    (C) 1995 AROS - The Amiga Replacement OS
    $Id$	 $Log

    Desc: Graphics function WritePixel()
    Lang: english
*/
#include "graphics_intern.h"
#include <graphics/rastport.h>

/*****************************************************************************

    NAME */
#include <graphics/rastport.h>
#include <proto/graphics.h>

	AROS_LH3(LONG, WritePixel,

/*  SYNOPSIS */
	AROS_LHA(struct RastPort *, rp, A1),
	AROS_LHA(LONG             , x, D0),
	AROS_LHA(LONG             , y, D1),

/*  LOCATION */
	struct GfxBase *, GfxBase, 54, Graphics)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    graphics_lib.fd and clib/graphics_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct GfxBase *,GfxBase)

    return driver_WritePixel (rp, x, y, GfxBase);

    AROS_LIBFUNC_EXIT
} /* WritePixel */
