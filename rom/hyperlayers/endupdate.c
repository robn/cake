/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include <aros/libcall.h>
#include <graphics/clip.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <graphics/layers.h>
#include "layers_intern.h"
#include "basicfuncs.h"
#include <proto/layers.h>

/*****************************************************************************

    NAME */

	AROS_LH2(void, EndUpdate,

/*  SYNOPSIS */
	AROS_LHA(struct Layer *, l    , A0),
	AROS_LHA(UWORD         , flag , D0),

/*  LOCATION */
	struct LayersBase *, LayersBase, 14, Layers)

/*  FUNCTION
        After the damaged areas are update this routine should be
        called so the regular cliprects of the layer can be installed.
        

    INPUTS
        l    -  pointer to layer
        flag -  TRUE if the update was complete. The damage list is disposed.
                FALSE it the update was partial. The damage list is kept. 

    RESULT

    NOTES

    EXAMPLE

    BUGS
      not tested

    SEE ALSO
         BeginUpdate()

    INTERNALS

    HISTORY
	27-11-96    digulla automatically created from
			    layers_lib.fd and clib/layers_protos.h

*****************************************************************************/
{
  AROS_LIBFUNC_INIT
  AROS_LIBBASE_EXT_DECL(struct LayersBase *,LayersBase)

  /*
  ** must reset the flag NOW !! Keep this order!!
  */
  l->Flags &= ~LAYERUPDATING;

  if (NULL != l->ClipRect)
  {
    if (IS_SMARTREFRESH(l))
      _CopyClipRectsToClipRects(l, 
                                l->ClipRect, 
                                l->cr2,
                                0,
				0,
                                FALSE,
                                TRUE,
				FALSE);
    else
      _FreeClipRectListBM(l,l->ClipRect);
  }
      
  l->ClipRect = l->cr2;
  l->cr2 = NULL;

  if (FALSE != flag)
  {
    /* the update was complete so I free the damage list */
    ClearRegion(l->DamageList);
    l->Flags &= ~LAYERREFRESH;
  }
  
  UnlockLayer(l);

  AROS_LIBFUNC_EXIT
} /* EndUpdate */
