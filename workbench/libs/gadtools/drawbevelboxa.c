/*
    (C) 1997 AROS - The Amiga Replacement OS
    $Id$

    Desc: Draw a bevelled box.
    Lang: english
*/
#include <proto/exec.h>
#include <intuition/classusr.h>
#include <intuition/imageclass.h>
#include <intuition/screens.h>
#include "gadtools_intern.h"

/*********************************************************************

    NAME */
#include <proto/gadtools.h>
#include <libraries/gadtools.h>
#include <graphics/rastport.h>
#include <utility/tagitem.h>

	AROS_LH6(void, DrawBevelBoxA,

/*  SYNOPSIS */
	AROS_LHA(struct RastPort *, rport, A0),
	AROS_LHA(WORD, left, D0),
	AROS_LHA(WORD, top, D1),
	AROS_LHA(WORD, width, D2),
	AROS_LHA(WORD, height, D3),
	AROS_LHA(struct TagItem *, taglist, A1),

/*  LOCATION */
	struct Library *, GadToolsBase, 20, GadTools)

/*  FUNCTION
	DrawBevelBoxA() does just that. It draws a bevelled box.

    INPUTS
	rport   - rastport, in which the box should be drawn
	left    - left edge of the box
	top     - top edge of the box
	width   - width of the box
	height  - height og the box
	taglist - additional tags

    RESULT

    NOTES
	Boxes drawn with DrawBevelBox() aren't refreshed automatically.
	You have to refresh them yourself.
	The rastport will be modified for DrawBevelBoxA() to guarantee
	fast drawing.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

***************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct GadToolsBase *,GadToolsBase)
    struct VisualInfo *vi;
    struct TagItem tags[5];

    vi = (struct VisualInfo *)GetTagData(GT_VisualInfo, NULL, taglist);
    if (vi == NULL)
	return;

    tags[0].ti_Tag = IA_Width;
    tags[0].ti_Data = width;
    tags[1].ti_Tag = IA_Height;
    tags[1].ti_Data = height;
    tags[2].ti_Tag = IA_Recessed;
    tags[2].ti_Data = GetTagData(GTBB_Recessed, FALSE, taglist);
    tags[3].ti_Tag = IA_FrameType;
    tags[3].ti_Data = GetTagData(GTBB_FrameType, BBFT_BUTTON, taglist);
    tags[4].ti_Tag = TAG_DONE;
    ObtainSemaphore(&((struct GadToolsBase_intern *)GadToolsBase)->bevelsema);
    SetAttrsA(((struct GadToolsBase_intern *)GadToolsBase)->bevel, tags);
    DrawImageState(rport, ((struct GadToolsBase_intern *)GadToolsBase)->bevel,
                   left, top,
                   IDS_NORMAL, vi->vi_dri);
    ReleaseSemaphore(&((struct GadToolsBase_intern *)GadToolsBase)->bevelsema);

    AROS_LIBFUNC_EXIT
} /* DrawBevelBoxA */
