/*
    (C) 1995 AROS - The Amiga Research OS
    $Id$	$Log

    Desc: AROS Graphics function DeinitRastPort()
    Lang: english
*/
#include "graphics_intern.h"
#include <graphics/rastport.h>

/*****************************************************************************

    NAME */
#include <proto/graphics.h>

	AROS_LH1(void, DeinitRastPort,

/*  SYNOPSIS */
	AROS_LHA(struct RastPort *, rp, A1),

/*  LOCATION */
	struct GfxBase *, GfxBase, 179, Graphics)

/*  FUNCTION
	Frees the contents of a RastPort structure. The structure itself
	is not freed.

    INPUTS
	rp - The RastPort which contents are to be freed.

    RESULT
	None.

    NOTES
	You can initialize the RastPort again via InitRastPort() but
	you must not use any other graphics function with it before
	that.

    EXAMPLE

    BUGS

    SEE ALSO
	InitRastPort()

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    graphics_lib.fd and clib/graphics_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct GfxBase *,GfxBase)

    driver_DeinitRastPort (rp, GfxBase);

    AROS_LIBFUNC_EXIT
} /* DeinitRastPort */
