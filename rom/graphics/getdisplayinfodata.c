/*
    (C) 2000 AROS - The Amiga Research OS
    $Id$

    Desc: Graphics function GetDisplayInfoData()
    Lang: english
*/
#include <graphics/displayinfo.h>

/*****************************************************************************

    NAME */
#include <proto/graphics.h>

        AROS_LH5(ULONG, GetDisplayInfoData,

/*  SYNOPSIS */
        AROS_LHA(DisplayInfoHandle, handle, A0),
        AROS_LHA(UBYTE *, buf, A1),
        AROS_LHA(ULONG, size, D0),
        AROS_LHA(ULONG, tagID, D1),
        AROS_LHA(ULONG, ID, D2),

/*  LOCATION */
        struct GfxBase *, GfxBase, 126, Graphics)

/*  FUNCTION

    INPUTS
        handle - displayinfo handle
        buf    - pointer to destination buffer
        size   - buffer size in bytes
        tagID  - data chunk type
        ID     - displayinfo identifier, optionally used if handle is NULL

    RESULT
        result - if positive, number of bytes actually transferred
                 if zero, no information for ID was available

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        FindDisplayInfo() NextDisplayInfo() graphics/displayinfo.h

    INTERNALS

    HISTORY


******************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct GfxBase *,GfxBase)

#warning TODO: Write graphics/GetDisplayInfoData()
    aros_print_not_implemented ("GetDisplayInfoData");

    return 0;

    AROS_LIBFUNC_EXIT
} /* GetDisplayInfoData */
