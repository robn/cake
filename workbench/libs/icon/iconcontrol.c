/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <exec/types.h>
#include <workbench/icon.h>
#include <utility/tagitem.h>
#include <proto/icon.h>

#include "icon_intern.h"

#   include <aros/debug.h>

/*****************************************************************************

    NAME */

    AROS_LH2(ULONG, IconControlA,

/*  SYNOPSIS */
        AROS_LHA(struct DiskObject *, icon, A0),
        AROS_LHA(struct TagItem *,    tags, A1),

/*  LOCATION */
        struct Library *, IconBase, 26, Icon)

/*  FUNCTION
	Set and get icon and icon.library options.
	
    INPUTS
	icon - icon to be queried

    RESULT
	Number of processed tags.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    
    const struct TagItem *tstate       = tags;
    struct TagItem       *tag          = NULL;
    ULONG                 processed    = 0;

    LONG                 *errorCode    = NULL;
    struct TagItem      **errorTagItem = NULL;
    struct NativeIcon 	 *nativeicon = NULL;
    
    if (icon)
    {
    	nativeicon = GetNativeIcon(icon, LB(IconBase));
    }
    
#   define STORE(pointer, value)   (pointer != NULL ? *pointer = (value) : (value))
#   define SET_ERRORCODE(value)    STORE(errorCode, (value))
#   define SET_ERRORTAGITEM(value) STORE(errorTagItem, (value))

    /* The following tags need to be setup early ---------------------------*/
    errorCode = (LONG *) GetTagData(ICONA_ErrorCode, 0, tags);
    SET_ERRORCODE(0);
    
    errorTagItem = (struct TagItem **) GetTagData(ICONA_ErrorTagItem, 0, tags);
    SET_ERRORTAGITEM(NULL);

    /* Parse taglist -------------------------------------------------------*/
    while ((tag = NextTagItem(&tstate)) != NULL)
    {
        switch (tag->ti_Tag)
        {
            /* Global tags -------------------------------------------------*/
            case ICONCTRLA_SetGlobalScreen:
                LB(IconBase)->ib_Screen = (struct Screen *) tag->ti_Data;
                processed++;
                break;
                
            case ICONCTRLA_GetGlobalScreen:
                STORE((struct Screen **) tag->ti_Data, LB(IconBase)->ib_Screen);
                processed++;
                break;
                
            case ICONCTRLA_SetGlobalPrecision:
            case OBP_Precision:
                LB(IconBase)->ib_Precision = tag->ti_Data;
                processed++;
                break;
                
            case ICONCTRLA_GetGlobalPrecision:
                STORE((LONG *) tag->ti_Data, LB(IconBase)->ib_Precision);
                processed++;
                break;
                
            case ICONCTRLA_SetGlobalEmbossRect:
                // FIXME
                break;
                
            case ICONCTRLA_GetGlobalEmbossRect:
                // FIXME
                break;
                
            case ICONCTRLA_SetGlobalFrameless:
                LB(IconBase)->ib_Frameless = tag->ti_Data;
                processed++;
                break;
                
            case ICONCTRLA_GetGlobalFrameless:
                STORE((LONG *) tag->ti_Data, LB(IconBase)->ib_Frameless);
                processed++;
                break;
                
            case ICONCTRLA_SetGlobalIdentifyHook:
                LB(IconBase)->ib_IdentifyHook = (struct Hook *) tag->ti_Data;
                processed++;
                break;
                
            case ICONCTRLA_GetGlobalIdentifyHook:
                STORE((struct Hook **) tag->ti_Data, LB(IconBase)->ib_IdentifyHook);
                processed++;
                break;
                
            case ICONCTRLA_SetGlobalMaxNameLength:
                LB(IconBase)->ib_MaxNameLength = tag->ti_Data;
                processed++;
                break;
            
            case ICONCTRLA_GetGlobalMaxNameLength:
                STORE((LONG *) tag->ti_Data, LB(IconBase)->ib_MaxNameLength);
                processed++;
                break;
                
            case ICONCTRLA_SetGlobalNewIconsSupport:
                LB(IconBase)->ib_NewIconsSupport = tag->ti_Data;
                processed++;
                break;
                
            case ICONCTRLA_GetGlobalNewIconsSupport:
                STORE((LONG *) tag->ti_Data, LB(IconBase)->ib_NewIconsSupport);
                processed++;
                break;
                
            case ICONCTRLA_SetGlobalColorIconSupport:
                LB(IconBase)->ib_ColorIconSupport = tag->ti_Data;
                processed++;
                break;
                
            case ICONCTRLA_GetGlobalColorIconSupport:
                STORE((LONG *) tag->ti_Data, LB(IconBase)->ib_ColorIconSupport);
                processed++;
                break;
            
            
            /* Local tags --------------------------------------------------*/
            case ICONCTRLA_GetImageMask1:
	    	if (nativeicon)
		{
		    STORE((PLANEPTR *)tag->ti_Data, (PLANEPTR)nativeicon->icon35.img1.mask);
		}
		else
		{
		    STORE((PLANEPTR *)tag->ti_Data, 0);
		}
		processed++;
                break;
                
            case ICONCTRLA_GetImageMask2:
	    	if (nativeicon)
		{
		    STORE((PLANEPTR *)tag->ti_Data, (PLANEPTR)nativeicon->icon35.img2.mask);
		}
		else
		{
		    STORE((PLANEPTR *)tag->ti_Data, 0);
		}
		processed++;
                break;
                
            case ICONCTRLA_SetTransparentColor1:
                break;
                
            case ICONCTRLA_GetTransparentColor1:
	    	if (nativeicon && (nativeicon->icon35.img1.flags & IMAGE35F_HASTRANSPARENTCOLOR))
		{
		    STORE((LONG *)tag->ti_Data, (LONG)nativeicon->icon35.img1.transparentcolor);
		}
		else
		{
		    STORE((LONG *)tag->ti_Data, -1);
		}
		processed++;
                break;
                
            case ICONCTRLA_SetTransparentColor2:
                break;
                
            case ICONCTRLA_GetTransparentColor2:
	    	if (nativeicon && (nativeicon->icon35.img2.flags & IMAGE35F_HASTRANSPARENTCOLOR))
		{
		    STORE((LONG *)tag->ti_Data, (LONG)nativeicon->icon35.img2.transparentcolor);
		}
		else
		{
		    STORE((LONG *)tag->ti_Data, -1);
		}
		processed++;
                break;
                
            case ICONCTRLA_SetPalette1:
                break;
                
            case ICONCTRLA_GetPalette1:
	    	if (nativeicon)
		{
		    STORE((struct ColorRegister **)tag->ti_Data, (struct ColorRegister *)nativeicon->icon35.img1.palette);
		}
		else
		{
		    STORE((struct ColorRegister **)tag->ti_Data, 0);
		}
		processed++;
                break;
                
            case ICONCTRLA_SetPalette2:
                break;
                
            case ICONCTRLA_GetPalette2:
	    	if (nativeicon)
		{
		    STORE((struct ColorRegister **)tag->ti_Data, (struct ColorRegister *)nativeicon->icon35.img2.palette);
		}
		else
		{
		    STORE((struct ColorRegister **)tag->ti_Data, 0);
		}
		processed++;
                break;
                
            case ICONCTRLA_SetPaletteSize1:
                break;
                
            case ICONCTRLA_GetPaletteSize1:
	    	if (nativeicon)
		{
		    STORE((ULONG *)tag->ti_Data, nativeicon->icon35.img1.numcolors);
		}
		else
		{
		    STORE((ULONG *)tag->ti_Data, 0);
		}
		processed++;
                break;
                
            case ICONCTRLA_SetPaletteSize2:
                break;
                
            case ICONCTRLA_GetPaletteSize2:
	    	if (nativeicon)
		{
		    STORE((ULONG *)tag->ti_Data, nativeicon->icon35.img2.numcolors);
		}
		else
		{
		    STORE((ULONG *)tag->ti_Data, 0);
		}
		processed++;
                break;
                
            case ICONCTRLA_SetImageData1:
                break;
                
            case ICONCTRLA_GetImageData1:
	    	if (nativeicon)
		{
		    STORE((UBYTE **)tag->ti_Data, nativeicon->icon35.img1.imagedata);
		}
		else
		{
		    STORE((UBYTE **)tag->ti_Data, 0);
		}
		processed++;
                break;
                
            case ICONCTRLA_SetImageData2:
                break;
                
            case ICONCTRLA_GetImageData2:
	    	if (nativeicon)
		{
		    STORE((UBYTE **)tag->ti_Data, nativeicon->icon35.img2.imagedata);
		}
		else
		{
		    STORE((UBYTE **)tag->ti_Data, 0);
		}
		processed++;
                break;
                
            case ICONCTRLA_SetFrameless:
                break;
                
            case ICONCTRLA_GetFrameless:
                break;
                
            case ICONCTRLA_SetNewIconsSupport:
                break;
                
            case ICONCTRLA_GetNewIconsSupport:
                break;
                
            case ICONCTRLA_SetAspectRatio:
                break;
                
            case ICONCTRLA_GetAspectRatio:
                break;
                
            case ICONCTRLA_SetWidth:
                break;
                
            case ICONCTRLA_GetWidth:
		processed++;
	    	if (nativeicon)
		{
		    if (nativeicon->iconPNG.img1)
		    {
		    	STORE((ULONG *)tag->ti_Data, nativeicon->iconPNG.width);
			break;
		    }
		    
		    if (nativeicon->icon35.img1.imagedata)
		    {
		    	STORE((ULONG *)tag->ti_Data, nativeicon->icon35.width);
			break;
		    }			
		}
		
		if (icon)
		{
		    STORE((ULONG *)tag->ti_Data, icon->do_Gadget.Width);
		}
		else
		{
		    STORE((ULONG *)tag->ti_Data, 0);
		}
                break;
                
            case ICONCTRLA_SetHeight:
                break;
                
            case ICONCTRLA_GetHeight:
		processed++;
	    	if (nativeicon)
		{
		    if (nativeicon->iconPNG.img1)
		    {
		    	STORE((ULONG *)tag->ti_Data, nativeicon->iconPNG.height);
			break;
		    }
		    
		    if (nativeicon->icon35.img1.imagedata)
		    {
		    	STORE((ULONG *)tag->ti_Data, nativeicon->icon35.height);
			break;
		    }			
		}
		
		if (icon)
		{
		    STORE((ULONG *)tag->ti_Data, icon->do_Gadget.Height);
		}
		else
		{
		    STORE((ULONG *)tag->ti_Data, 0);
		}
                break;
                
            case ICONCTRLA_IsPaletteMapped:
	    	if (nativeicon && nativeicon->icon35.img1.imagedata)
		{
		    STORE((LONG *)tag->ti_Data, 1);
		}
		else
		{
		    STORE((LONG *)tag->ti_Data, 0);
		}
		processed++;				
                break;
                
            case ICONCTRLA_IsNewIcon:
                break;
                
            case ICONCTRLA_IsNativeIcon:
	    	if (nativeicon)
		{
		    STORE((LONG *)tag->ti_Data, 1);
		}
		else
		{
		    STORE((LONG *)tag->ti_Data, 0);
		}
		processed++;
                break;
                
            case ICONGETA_IsDefaultIcon:
                break;
                
            case ICONCTRLA_GetScreen:
                break;
                
            case ICONCTRLA_HasRealImage2:
                break;
		
	    case ICONCTRLA_GetARGBImageData1:
	    	if (nativeicon)
		{
		    STORE((ULONG **)tag->ti_Data, (ULONG *)nativeicon->iconPNG.img1);
		}
		else
		{
		    STORE((ULONG **)tag->ti_Data, 0);
		}
		processed++;
	    	break;

	    case ICONCTRLA_GetARGBImageData2:
	    	if (nativeicon)
		{
		    STORE((ULONG **)tag->ti_Data, (ULONG *)nativeicon->iconPNG.img2);
		}
		else
		{
		    STORE((ULONG **)tag->ti_Data, 0);
		}
		processed++;
	    	break;
		
        }
    }
    
    return processed;
    
    AROS_LIBFUNC_EXIT
    
#   undef STORE
#   undef SET_ERRORCODE
#   undef SET_ERRORTAGITEM
    
} /* IconControlA() */
