/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.1  1996/10/10 13:09:55  digulla
    New function: NewModifyProp()


    Desc:
    Lang: english
*/
#include "intuition_intern.h"
#include "propgadgets.h"

/*****************************************************************************

    NAME */
	#include <intuition/intuition.h>
	#include <clib/intuition_protos.h>

	__AROS_LH9(void, NewModifyProp,

/*  SYNOPSIS */
	__AROS_LHA(struct Gadget    *, gadget, A0),
	__AROS_LHA(struct Window    *, window, A1),
	__AROS_LHA(struct Requester *, requester, A2),
	__AROS_LHA(unsigned long     , flags, D0),
	__AROS_LHA(unsigned long     , horizPot, D1),
	__AROS_LHA(unsigned long     , vertPot, D2),
	__AROS_LHA(unsigned long     , horizBody, D3),
	__AROS_LHA(unsigned long     , vertBody, D4),
	__AROS_LHA(long              , numGad, D5),

/*  LOCATION */
	struct IntuitionBase *, IntuitionBase, 78, Intuition)

/*  FUNCTION
	Changes the values in the PropInfo-structure of a proportional
	gadget and refreshes the specified number of gadgets beginning
	at the proportional gadget. If numGad is 0 (zero), then no
	refreshing is done.

    INPUTS
	gadget - Must be a PROPGADGET
	window - The window which contains the gadget
	requester - If the gadget has GTYP_REQGADGET set, this must be
		non-NULL.
	flags - New flags
	horizPot - New value for the HorizPot field of the PropInfo
	vertPot - New value for the VertPot field of the PropInfo
	horizBody - New value for the HorizBody field of the PropInfo
	vertBody - New value for the VertBody field of the PropInfo
	numGad - How many gadgets to refresh. 0 means none (not even
		the current gadget) and -1 means all of them.

    RESULT
	None.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	NewModifyProp(), RefreshGadgets(), RefreshGList()

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    intuition_lib.fd and clib/intuition_protos.h

*****************************************************************************/
{
    __AROS_FUNC_INIT
    __AROS_BASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)
    struct PropInfo * pi;
    struct BBox old, new;
    int right, bottom;

    if ((gadget->GadgetType & GTYP_GTYPEMASK) != GTYP_PROPGADGET
	|| !gadget->SpecialInfo
    )
	return;

    pi = gadget->SpecialInfo;

    CalcBBox (window, gadget, &old);

    new = old;

    CalcKnobSize (gadget, &old);

    pi->Flags = flags;
    pi->HorizPot = horizPot;
    pi->VertPot = vertPot;
    pi->HorizBody = horizBody;
    pi->VertBody = vertBody;

    CalcKnobSize (gadget, &new);

    right = old.Left + old.Width; /* No -1 here; we don't add +1 later */
    bottom = old.Top + old.Height;

    /* Calculate area to clear */
    if (new.Left < old.Left)
	old.Left = new.Left;

    if (new.Top < old.Top)
	old.Top = new.Top;

    if (new.Left+new.Width > right)
	right = new.Left + new.Width;

    if (new.Top+new.Height > bottom)
	bottom = new.Top + new.Height;

    old.Width = right - old.Left; /* No +1 here; see above */
    old.Height = bottom - old.Top; /* No +1 here; see above */

    RefreshPropGadgetKnob (flags, &old, &new, window, IntuitionBase);

    if (numGad > 1 && gadget->NextGadget)
	RefreshGList (gadget->NextGadget, window, requester, numGad-1);

    __AROS_FUNC_EXIT
} /* NewModifyProp */
