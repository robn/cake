/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include <aros/libcall.h>

/*****************************************************************************

    NAME */

	AROS_LH2(void, SwapBitsRastPortClipRect,

/*  SYNOPSIS */
	AROS_LHA(struct RastPort *, rp, A0),
	AROS_LHA(struct ClipRect *, cr, A1),

/*  LOCATION */
	struct LayersBase *, LayersBase, 40, Layers)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
  AROS_LIBFUNC_INIT
  AROS_LIBBASE_EXT_DECL(struct LayersBase *,LayersBase)

  /* 
     First allocate a BitMap where the BitMap Data of the RastPort's 
     bitmap will go into. 
  */
  struct BitMap * NewBM;
  
  NewBM = AllocBitMap(GetBitMapAttr(cr->BitMap, BMA_WIDTH) + 16,
                      GetBitMapAttr(cr->BitMap, BMA_HEIGHT),
                      GetBitMapAttr(cr->BitMap, BMA_DEPTH),
                      0,
                      rp->BitMap);
  /*
     Save the displayed bitmap area to the new BitMap
   */

  BltBitMap(rp->BitMap,
            cr->bounds.MinX,
            cr->bounds.MinY,
            NewBM,
            cr->bounds.MinX & 0x0f,
            0,
            cr->bounds.MaxX - cr->bounds.MinX + 1,
            cr->bounds.MaxY - cr->bounds.MinY + 1,
            0x0c0,
            0xff,
            NULL
           );
  
  /*
     Display the contents of the ClipRect's BitMap.
   */  

  BltBitMap(cr->BitMap,
            cr->bounds.MinX & 0x0f,
            0,
            rp->BitMap,
            cr->bounds.MinX,
            cr->bounds.MinY,
            cr->bounds.MaxX - cr->bounds.MinX + 1,
            cr->bounds.MaxY - cr->bounds.MinY + 1,
            0x0c0,
            0xff,
            NULL
           );

  /*
     Free the 'old' BitMap of the ClipRect and hang the new BitMap into
     the ClipRect structure.
   */
  FreeBitMap(cr->BitMap);
  cr->BitMap = NewBM;

  AROS_LIBFUNC_EXIT
} /* SwapBitsRastPortClipRect */
