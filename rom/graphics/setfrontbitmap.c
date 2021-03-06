/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Private graphics function for allocating screen bitmaps
    Lang: english
*/
#include <aros/debug.h>
#include "graphics_intern.h"
#include "gfxfuncsupport.h"
#include <exec/memory.h>
#include <graphics/rastport.h>
#include <proto/exec.h>
#include <proto/oop.h>
#include <oop/oop.h>

/*****************************************************************************

    NAME */
#include <graphics/rastport.h>
#include <proto/graphics.h>

	AROS_LH2(BOOL , SetFrontBitMap,

/*  SYNOPSIS */
	AROS_LHA(struct BitMap *, bitmap, A0),
	AROS_LHA(BOOL, copyback, D0),

/*  LOCATION */
	struct GfxBase *, GfxBase, 184, Graphics)

/*  FUNCTION
	Sets the supplied screen as the frontmost, e.g. shows it in the display.

    INPUTS
	bitmap - The bitmap to put in front. Must be a displayable bitmap.
	copyback - Whether to copy back from the framebuffer into
	           the previously front bitmap. !!!! Only set this to TRUE 
		   this if you are 100% SURE that
		   the previously shown bitmap has not been disposed

    RESULT
    	success - TRUE if successful, FALSE otherwise.

    NOTES
	This function is private and AROS specific.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    
    #warning THIS IS NOT THREADSAFE
    
    /* To make this threadsafe we have to lock
       all gfx access in all the rendering calls
    */
    OOP_Object      	*cmap, *pf;
    HIDDT_ColorModel 	colmod;
    OOP_Object      	*fb;
    BOOL    	    	ok = FALSE;
    ULONG   	    	showflags = 0;

    //if (bitmap && (BMF_DISPLAYABLE != (bitmap->Flags & BMF_DISPLAYABLE)))
    if (bitmap && (!(bitmap->Flags & BMF_AROS_HIDD) || !(HIDD_BM_FLAGS(bitmap) & HIDD_BMF_SCREEN_BITMAP)))
    {
    	D(bug("!!! SetFrontBitMap: TRYING TO SET NON-DISPLAYABLE BITMAP !!!\n"));
	return FALSE;
    }
    
    if ( SDD(GfxBase)->frontbm == bitmap)
    {
    	D(bug("!!!!!!!!!!!!!!! SHOWING BITMAP %p TWICE !!!!!!!!!!!\n", bitmap));
	return TRUE;
    }
    
    if (copyback)
    {
    	showflags |= fHidd_Gfx_Show_CopyBack;
    }
    
    fb = HIDD_Gfx_Show(SDD(GfxBase)->gfxhidd, (bitmap ? HIDD_BM_OBJ(bitmap) : NULL), showflags);
    
    if (NULL == fb)
    {
    	D(bug("!!! SetFrontBitMap: HIDD_Gfx_Show() FAILED !!!\n"));
    }
    else
    {
	Forbid();
	
	 /* Set this as the active screen */
    	if (NULL != SDD(GfxBase)->frontbm)
	{
    	    struct BitMap *oldbm;
    	    /* Put back the old values into the old bitmap */
	    oldbm = SDD(GfxBase)->frontbm;
	    HIDD_BM_OBJ(oldbm)		= SDD(GfxBase)->bm_bak;
	    HIDD_BM_COLMOD(oldbm)	= SDD(GfxBase)->colmod_bak;
	    HIDD_BM_COLMAP(oldbm)	= SDD(GfxBase)->colmap_bak;
	}
	
    
	SDD(GfxBase)->frontbm		= bitmap;
	SDD(GfxBase)->bm_bak		= bitmap ? HIDD_BM_OBJ(bitmap) : NULL;
	SDD(GfxBase)->colmod_bak	= bitmap ? HIDD_BM_COLMOD(bitmap) : NULL;
	SDD(GfxBase)->colmap_bak	= bitmap ? HIDD_BM_COLMAP(bitmap) : NULL;
    
	if (bitmap)
	{
	    IPTR width, height;
	    
	    /* Insert the framebuffer in its place */
	    OOP_GetAttr(fb, aHidd_BitMap_ColorMap, (IPTR *)&cmap);
	    OOP_GetAttr(fb, aHidd_BitMap_PixFmt, (IPTR *)&pf);
	    OOP_GetAttr(pf, aHidd_PixFmt_ColorModel, &colmod);

	    HIDD_BM_OBJ(bitmap)	= fb;
	    HIDD_BM_COLMOD(bitmap)	= colmod;
	    HIDD_BM_COLMAP(bitmap)	= cmap;
  
    	#if 1 /* CHECKME! */
            OOP_GetAttr(SDD(GfxBase)->bm_bak, aHidd_BitMap_Width, &width);
    	    OOP_GetAttr(SDD(GfxBase)->bm_bak, aHidd_BitMap_Height, &height);
	   
	    HIDD_BM_UpdateRect(fb, 0, 0, width, height);
	#endif
    	    
	}
	Permit();
	
	ok = TRUE;
    }
    
    return ok;

    AROS_LIBFUNC_EXIT
    
} /* AllocScreenBitMap */
