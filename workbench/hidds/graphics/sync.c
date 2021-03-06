/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Sync info class
    Lang: English.
*/

/****************************************************************************************/

#include <proto/oop.h>
#include <proto/utility.h>
#include <exec/memory.h>
#include <oop/oop.h>
#include <utility/tagitem.h>
#include <hidd/graphics.h>

#define DEBUG 0
#include <aros/debug.h>

#include "graphics_intern.h"

/****************************************************************************************/

OOP_Object *Sync__Root__New(OOP_Class *cl, OOP_Object *o, struct pRoot_New *msg)
{
    struct sync_data 	*data;
    BOOL    	    	ok = FALSE;
    
    DECLARE_ATTRCHECK(sync);
    
    EnterFunc(bug("Sync::New()\n"));
    
    /* Get object from superclass */
    o = (OOP_Object *)OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
    if (NULL == o)
	return NULL;

    /* If we got a NULL attrlist we just allocate an empty object an exit */
    if (NULL == msg->attrList)
	return o;

    data = OOP_INST_DATA(cl, o);
    
    if (!parse_sync_tags(msg->attrList, data, ATTRCHECK(sync), CSD(cl) ))
    {
	D(bug("!!! ERROR PARSING SYNC ATTRS IN Sync::New() !!!\n"));
    }
    else
    {
	ok = TRUE;
    }
    
    if (!ok)
    {
	OOP_MethodID dispose_mid;
	
	dispose_mid = OOP_GetMethodID(IID_Root, moRoot_Dispose);
	OOP_CoerceMethod(cl, o, (OOP_Msg)&dispose_mid);
	o = NULL;
    }
    
    return o;
}

/****************************************************************************************/

VOID Sync__Root__Get(OOP_Class *cl, OOP_Object *o, struct pRoot_Get *msg)
{
    struct sync_data 	*data;    
    ULONG   	    	idx;
    
    data = OOP_INST_DATA(cl, o);
    
    if (IS_SYNC_ATTR(msg->attrID, idx))
    {
    	switch (idx)
	{
	    case aoHidd_Sync_PixelTime:
		*msg->storage = (IPTR)data->pixtime;
		break;
		
	    case aoHidd_Sync_PixelClock:
	    {
    	    #if AROS_NOFPU
    	    	#warning Find code for non-FPU!
		*msg->storage = (ULONG)0x12345678;
    	    #else
		DOUBLE pixtime, pixclock;
		
		pixtime = (DOUBLE)data->pixtime;
		
		pixtime /= 1000000000000;	/* pixtime is in 10E-12 secs */
		pixclock = 1 / pixtime;		/* convert to Hz */
		*msg->storage = (ULONG)pixclock;
    	    #endif
		break;
	    }
		
	    case aoHidd_Sync_LeftMargin:
		*msg->storage = (IPTR)data->left_margin;
		break;
		
	    case aoHidd_Sync_RightMargin:
		*msg->storage = (IPTR)data->right_margin;
		break;
		
	    case aoHidd_Sync_HSyncLength:
		*msg->storage = (IPTR)data->hsync_length;
		break;
		
	    case aoHidd_Sync_UpperMargin:
		*msg->storage = (IPTR)data->upper_margin;
		break;
		
	    case aoHidd_Sync_LowerMargin:
		*msg->storage = (IPTR)data->lower_margin;
		break;
		
	    case aoHidd_Sync_VSyncLength:
		*msg->storage = (IPTR)data->vsync_length;
		break;
		
	    case aoHidd_Sync_HDisp:
		*msg->storage = (IPTR)data->hdisp;
		break;
		
	    case aoHidd_Sync_VDisp:
		*msg->storage = (IPTR)data->vdisp;
		break;
		
	    case aoHidd_Sync_HSyncStart:
		*msg->storage = (IPTR)(data->hdisp + data->right_margin);
		break;
		
	    case aoHidd_Sync_HSyncEnd:
		*msg->storage = (IPTR)(data->hdisp + data->right_margin + data->hsync_length);
		break;
		
	    case aoHidd_Sync_HTotal:
		*msg->storage = (IPTR)(data->hdisp + data->right_margin + data->hsync_length + data->left_margin);
		break;
		
	    case aoHidd_Sync_VSyncStart:
		*msg->storage = (IPTR)(data->vdisp + data->lower_margin);
		break;
		
	    case aoHidd_Sync_VSyncEnd:
		*msg->storage = (IPTR)(data->vdisp + data->lower_margin + data->vsync_length);
		break;
		
	    case aoHidd_Sync_VTotal:
		*msg->storage = (IPTR)(data->vdisp + data->lower_margin + data->vsync_length + data->upper_margin);
		break;
				
	    case aoHidd_Sync_Description:
	    	*msg->storage = (IPTR)data->description;
		break;
		
	    default:
	     	D(bug("!!! TRYING TO GET UNKNOWN ATTR FROM SYNC OBJECT !!!\n"));
    		OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
		break;

	}
    
    }
    else
    {
    	OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
    }
    
    return;
    
}

/****************************************************************************************/
