/*
    (C) 1995-98 AROS - The Amiga Research OS
    $Id$

    Desc: Driver for using gfxhidd for gfx output
    Lang: english
*/

#define AROS_USE_OOP

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <exec/memory.h>
#include <exec/alerts.h>
#include <dos/dos.h>
#include <utility/tagitem.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <devices/keymap.h>
#include <devices/input.h>

#include <proto/exec.h>
#include <proto/layers.h>

#include <proto/graphics.h>
#include <proto/arossupport.h>
/* #include <proto/alib.h> */
#include "intuition_intern.h"
#include "gadgets.h"

#undef GfxBase
#undef LayersBase

#include "graphics_internal.h"

#include <proto/intuition.h>

#undef DEBUG
#undef SDEBUG
#define SDEBUG 0
#define DEBUG 0
#include <aros/debug.h>


static BOOL createsysgads(struct Window *w, struct IntuitionBase *IntuitionBase);
static VOID disposesysgads(struct Window *w, struct IntuitionBase *IntuitionBase);



enum {
    DRAGBAR = 0,
    CLOSEGAD,
    DEPTHGAD,
    SIZEGAD,
    ZOOMGAD,
    NUM_SYSGADS
};
    
	
struct HiddIntWindow
{
    struct IntWindow window;
    
    /* Some direct-pointers to the window system gadgets 
       (They are put in windows glist too)
    */
    Object * sysgads[NUM_SYSGADS];

};

#define HIW(x) ((struct HiddIntWindow *)x)
#define SYSGAD(w, idx) (HIW(w)->sysgads[idx])

static struct GfxBase *GfxBase = NULL;
static struct IntuitionBase * IntuiBase;
static struct Library *LayersBase = NULL;




int intui_init (struct IntuitionBase * IntuitionBase)
{
    


#warning FIXME: this is a hack
    IntuiBase = IntuitionBase;
    
    return TRUE;
}



int intui_open (struct IntuitionBase * IntuitionBase)
{
    
    /* Hack */
    if (!GfxBase)
    {
    	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0);
    	if (!GfxBase)
    	    return FALSE;
    }	    
    
    if (!LayersBase)
    {
    	LayersBase = OpenLibrary("layers.library", 0);
    	if (!LayersBase)
    	    return FALSE;
    }	    


    return TRUE;
}

void intui_close (struct IntuitionBase * IntuitionBase)
{
    return;
}

void intui_expunge (struct IntuitionBase * IntuitionBase)
{
    return;
}

void intui_SetWindowTitles (struct Window * win, UBYTE * text, UBYTE * screen)
{
}

int intui_GetWindowSize (void)
{
    return sizeof (struct HiddIntWindow);
}


#define TITLEBAR_HEIGHT 14
int intui_OpenWindow (struct Window * w,
	struct IntuitionBase * IntuitionBase,
	struct BitMap        * SuperBitMap)
{
    /* Create a layer for the window */
    LONG layerflags = 0;
    
    EnterFunc(bug("intui_OpenWindow(w=%p)\n", w));
    
    D(bug("screen: %p\n", w->WScreen));
    D(bug("bitmap: %p\n", w->WScreen->RastPort.BitMap));
    
    /* Just insert some default values, should be taken from
       w->WScreen->WBorxxxx */
    
    /* Set the layer's flags according to the flags of the
    ** window
    */
    
    /* refresh type */
    if (0 != (w->Flags & WFLG_SIMPLE_REFRESH))
      layerflags |= LAYERSIMPLE;
    else
      if (0!= (w->Flags & WFLG_SUPER_BITMAP))
        layerflags |= (LAYERSMART|LAYERSUPER);
      else
        layerflags |= LAYERSMART;
        
    if (0 != (w->Flags & WFLG_BACKDROP))   
      layerflags |= LAYERBACKDROP;

    D(bug("Window dims: (%d, %d, %d, %d)\n",
    	w->LeftEdge, w->TopEdge, w->Width, w->Height));
    	
    /* A GimmeZeroZero window??? */
    if (0 != (w->Flags & WFLG_GIMMEZEROZERO))
    {
      /* 
        A GimmeZeroZero window is to be created:
          - the outer window will be a simple refresh layer
          - the inner window will be a layer according to the flags
        What is the size of the inner/outer window supposed to be???
        I just make it that the inner window has the size of what is requested
      */ 
       
      /* First create outer window */
      struct Layer * L = CreateUpfrontHookLayer(
                             &w->WScreen->LayerInfo
                           , w->WScreen->RastPort.BitMap
                           , w->LeftEdge
                           , w->TopEdge
                           , w->LeftEdge + w->Width  + w->BorderLeft + w->BorderRight
                           , w->TopEdge  + w->Height + w->BorderTop  + w->BorderBottom
                           , LAYERSIMPLE | (layerflags & LAYERBACKDROP)
                           , LAYERS_NOBACKFILL
                           , SuperBitMap);
                           
      /* Could the layer be created. Nothing bad happened so far, so simply leave */
      if (NULL == L)
        ReturnBool("intui_OpenWindow", FALSE);

      /* install it as the BorderRPort */
      w->BorderRPort = L->rp;
       
      /* Now comes the inner window */
      w->WLayer = CreateUpfrontHookLayer( 
                   &w->WScreen->LayerInfo
	  	  , w->WScreen->RastPort.BitMap
		  , w->LeftEdge + w->BorderLeft
		  , w->TopEdge  + w->BorderTop
		  , w->LeftEdge + w->Width - 1
		  , w->TopEdge  + w->Height - 1
		  , layerflags
		  , LAYERS_BACKFILL
		  , SuperBitMap);

      /* could this layer be created? If not then delete the outer window and exit */
      if (NULL != w->WLayer)
      {
        DeleteLayer(0, L);
        ReturnBool("intui_OpenWindow", FALSE);
      }	
      	  
      /* That should do it, I guess... */
    }
    else
    {
      w->WLayer = CreateUpfrontHookLayer( 
                   &w->WScreen->LayerInfo
	  	  , w->WScreen->RastPort.BitMap
		  , w->LeftEdge
		  , w->TopEdge
		  , w->LeftEdge + w->Width - 1
		  , w->TopEdge  + w->Height - 1
		  , layerflags
		  , LAYERS_BACKFILL
		  , SuperBitMap);

      /* Install the BorderRPort here! see GZZ window above  */
      if (NULL != w->WLayer)
      {
        /* 
           I am installing a totally new RastPort here so window and frame can
           have different fonts etc. 
        */
        w->BorderRPort = AllocMem(sizeof(struct RastPort), MEMF_CLEAR);
        if (w->BorderRPort)
        {
          InitRastPort(w->BorderRPort);
          w->BorderRPort->Layer  = w->WLayer;
          w->BorderRPort->BitMap = w->WLayer->rp->BitMap;
        }
        else
        {
          /* no memory for RastPort! Simply close the window */
          intui_CloseWindow(w, IntuitionBase);
    	  ReturnBool("intui_OpenWindow", FALSE);
        }
      }		  
    }

    D(bug("Layer created: %p\n", w->WLayer));
    D(bug("Window created: %p\n", w));
    
    /* common code for GZZ and regular windows */
    
    if (w->WLayer)
    {
        /* Layer gets pointer to the window */
        w->WLayer->Window = (APTR)w;
	/* Window needs a rastport */
	w->RPort = w->WLayer->rp;
	
	/* installation of the correct BorderRPort already happened above !! */
	 
	if (createsysgads(w, IntuitionBase))
	{

    	    ReturnBool("intui_OpenWindow", TRUE);

	}
	intui_CloseWindow(w, IntuitionBase);
	
    } /* if (layer created) */
    
    ReturnBool("intui_OpenWindow", FALSE);
}

void intui_CloseWindow (struct Window * w,
	                struct IntuitionBase * IntuitionBase)
{
    disposesysgads(w, IntuitionBase);
    if (0 == (w->Flags & WFLG_GIMMEZEROZERO))
    {
      /* not a GZZ window */
      if (w->WLayer)
      	DeleteLayer(0, w->WLayer);
      DeinitRastPort(w->BorderRPort);
      FreeMem(w->BorderRPort, sizeof(struct RastPort));
    }
    else
    {
      /* a GZZ window */
      /* delete inner window */
      if (NULL != w->WLayer)
        DeleteLayer(0, w->WLayer);
      
      /* delete outer window */
      if (NULL != w->BorderRPort && 
          NULL != w->BorderRPort->Layer)
        DeleteLayer(0, w->BorderRPort->Layer);      
    }
}

void intui_RefreshWindowFrame(struct Window *w)
{
    /* Draw a frame around the window */
    struct RastPort *rp = w->BorderRPort;
    UWORD i;
    
    EnterFunc(bug("intui_RefreshWindowFrame(w=%p)\n", w));
    
    LockLayerRom(rp->Layer);
    
    SetAPen(rp, 1);
    D(bug("Pen set\n"));
    drawrect(rp, 0, 0, w->Width - 1, w->Height - 1, IntuitionBase);

    /* Refresh all the sytem gadgets */
    
    for (i = 0; i < NUM_SYSGADS; i ++)
    {
        if (SYSGAD(w, i))
	    RefreshGList((struct Gadget *)SYSGAD(w, i), w, NULL, 1 );
    }
    
    UnlockLayerRom(rp->Layer);
    
    ReturnVoid("intui_RefreshWindowFrame");
}

void intui_ChangeWindowBox (struct Window * window, WORD x, WORD y,
    WORD width, WORD height)
{
    if (0 != (window->Flags & WFLG_GIMMEZEROZERO))
    {
      MoveSizeLayer(window->BorderRPort->Layer,
    		   (x - window->TopEdge),
    		   (y - window->LeftEdge),
    		   (width - window->Width),
    		   (height - window->Height) );
      RefreshWindowFrame(window);
                    
    }
    MoveSizeLayer(window->WLayer,
    		  (x - window->TopEdge),
    		  (y - window->LeftEdge),
    		  (width - window->Width),
    		  (height - window->Height) );

    window->LeftEdge= x;
    window->TopEdge = y;
    window->Width   = width;
    window->Height  = height;

    RefreshWindowFrame(window);
}


void intui_WindowLimits (struct Window * win,
    WORD MinWidth, WORD MinHeight, UWORD MaxWidth, UWORD MaxHeight)
{

}

void intui_ActivateWindow (struct Window * win)
{

}

struct Window *intui_FindActiveWindow(struct InputEvent *ie, BOOL *swallow_event, struct IntuitionBase *IntuitionBase)
{
    /* The caller has checked that the input event is a IECLASS_RAWMOUSE, SELECTDOWN event */
    struct Screen *scr;
    struct Layer *l;
    struct Window *new_w = NULL;
    ULONG lock;
    
    *swallow_event = FALSE;

#warning Fixme: Find out what screen the click was in.

    lock = LockIBase(0UL);
    scr = IntuitionBase->ActiveScreen;
    UnlockIBase(lock);
    
    if (ie->ie_Class == IECLASS_RAWMOUSE && ie->ie_Code == SELECTDOWN)
    {
	/* What layer ? */
	D(bug("Click at (%d,%d)\n",ie->ie_X,ie->ie_Y));
	LockLayerInfo(&scr->LayerInfo);
	
	l = WhichLayer(&scr->LayerInfo, ie->ie_X, ie->ie_Y);
	
	UnlockLayerInfo(&scr->LayerInfo);
	
	if (!l)
	{
	    D(bug("iih: Click not inside layer\n"));
	}
	else
	{
	
	    new_w = (struct Window *)l->Window;
	    if (!new_w)
	    {
		D(bug("iih: Selected layer is not a window\n"));
	    }
    
    D(bug("Found layer %p\n", l));
    
        }
    
    
    }
    return new_w;

    
}

LONG intui_RawKeyConvert (struct InputEvent * ie, STRPTR buf,
	LONG size, struct KeyMap * km)
{

    return 0;
} /* intui_RawKeyConvert */

void intui_BeginRefresh (struct Window * win,
	struct IntuitionBase * IntuitionBase)
{
  /* lock all necessary layers */
  LockLayerRom(win->WLayer);
  /* Find out whether it's a GimmeZeroZero window with an extra layer to lock */
  if (0 != (win->Flags & WFLG_GIMMEZEROZERO))
    LockLayerRom(win->BorderRPort->Layer);

  /* I don't think I ever have to update the BorderRPort's layer */
  BeginUpdate(win->WLayer);
  
  /* let the user know that we're currently doing a refresh */
  win->Flags |= WFLG_WINDOWREFRESH;
  
} /* intui_BeginRefresh */

void intui_EndRefresh (struct Window * win, BOOL free,
	struct IntuitionBase * IntuitionBase)
{
  EndUpdate(win->WLayer, free);
  
  /* reset all bits indicating a necessary or ongoing refresh */
  win->Flags &= ~WFLG_WINDOWREFRESH;
  
  /* I reset this one only if Complete is TRUE!?! */
  if (TRUE == free)
    win->WLayer->Flags &= ~LAYERREFRESH;

  /* Unlock the layers. */
  if (0 != (win->Flags & WFLG_GIMMEZEROZERO))
    UnlockLayerRom(win->BorderRPort->Layer);
  
  UnlockLayerRom(win->WLayer);
 
} /* intui_EndRefresh */



static BOOL createsysgads(struct Window *w, struct IntuitionBase *IntuitionBase)
{

    struct DrawInfo *dri;
    EnterFunc(bug("createsysgads(w=%p)\n", w));
	
    dri = GetScreenDrawInfo(w->WScreen);
    if (dri)
    {
	LONG db_left, db_width, relright; /* dragbar sizes */
	BOOL sysgads_ok = TRUE;
	
	    
	db_left = 0; db_width = w->Width;
	
	
	
	
	/* Now find out what gadgets the window wants */
	if (    w->Flags & WFLG_CLOSEGADGET 
	     || w->Flags & WFLG_DEPTHGADGET
	     || w->Flags & WFLG_HASZOOM
	     || w->Flags & WFLG_DRAGBAR /* To assure w->BorderTop being set correctly */
	)
	{
	/* If any of titlebar gadgets are present, me might just as well
	insert a dragbar too */
	       
	    w->Flags |= WFLG_DRAGBAR;

	    w->BorderTop = TITLEBAR_HEIGHT;
	}
	
	/* Relright of rightmost button */
	relright = - (TITLEBAR_HEIGHT - 1);
	
	if (w->Flags & WFLG_DEPTHGADGET)
	{
	    struct TagItem depth_tags[] = {
	            {GA_RelRight,	relright	},
		    {GA_Top,		0  		},
		    {GA_Width,		TITLEBAR_HEIGHT	},
		    {GA_Height,		TITLEBAR_HEIGHT	},
		    {GA_DrawInfo,	(IPTR)dri 	},	/* required	*/
		    {GA_SysGadget,	TRUE		},
		    {GA_SysGType,	GTYP_WDEPTH 	},
		    {TAG_DONE,		0UL }
	    };
		
	    relright -= TITLEBAR_HEIGHT;
		
	    db_width -= TITLEBAR_HEIGHT;
	    
	    SYSGAD(w, DEPTHGAD) = NewObjectA(
			GetPrivIBase(IntuitionBase)->tbbclass
			, NULL
			, depth_tags );

	    if (!SYSGAD(w, DEPTHGAD))
		sysgads_ok = FALSE;
	    
	    
	}  

	if (w->Flags & WFLG_HASZOOM)
	{
	    struct TagItem zoom_tags[] = {
	            {GA_RelRight,	relright	},
		    {GA_Top,		0  		},
		    {GA_Width,		TITLEBAR_HEIGHT	},
		    {GA_Height,		TITLEBAR_HEIGHT	},
		    {GA_DrawInfo,	(IPTR)dri 	},	/* required	*/
		    {GA_SysGadget,	TRUE		},
		    {GA_SysGType,	GTYP_WZOOM 	},
		    {TAG_DONE,		0UL }
	    };
		
	    relright -= TITLEBAR_HEIGHT;
	    db_width -= TITLEBAR_HEIGHT;
	    
	    SYSGAD(w, ZOOMGAD) = NewObjectA(
			GetPrivIBase(IntuitionBase)->tbbclass
			, NULL
			, zoom_tags );

	    if (!SYSGAD(w, ZOOMGAD))
		sysgads_ok = FALSE;
	}  

	if (w->Flags & WFLG_CLOSEGADGET)
	{
	    struct TagItem close_tags[] = {
	            {GA_Left,		0		},
		    {GA_Top,		0  		},
		    {GA_Width,		TITLEBAR_HEIGHT	},
		    {GA_Height,		TITLEBAR_HEIGHT	},
		    {GA_DrawInfo,	(IPTR)dri 	},	/* required	*/
		    {GA_SysGadget,	TRUE		},
		    {GA_SysGType,	GTYP_CLOSE 	},
		    {TAG_DONE,		0UL }
	    };
		
	    db_left  += TITLEBAR_HEIGHT;
	    db_width -= TITLEBAR_HEIGHT;
	    
	    SYSGAD(w, CLOSEGAD) = NewObjectA(
			GetPrivIBase(IntuitionBase)->tbbclass
			, NULL
			, close_tags );

	    if (!SYSGAD(w, CLOSEGAD))
		sysgads_ok = FALSE;
	}  

	/* Now try to create the various gadgets */
	if (w->Flags & WFLG_DRAGBAR)
	{

	    struct TagItem dragbar_tags[] = {
			{GA_Left,	db_left		},
			{GA_Top,	0		},
			{GA_Width,	db_width 	},
			{GA_Height,	TITLEBAR_HEIGHT },
			{TAG_DONE,	0UL}
	    };
	    SYSGAD(w, DRAGBAR) = NewObjectA(
			GetPrivIBase(IntuitionBase)->dragbarclass
			, NULL
			, dragbar_tags );
				
	    if (!SYSGAD(w, DRAGBAR))
		sysgads_ok = FALSE;
				
	}
	    

	D(bug("Dragbar:  %p\n", SYSGAD(w, DRAGBAR ) ));
	D(bug("Depthgad: %p\n", SYSGAD(w, DEPTHGAD) ));
	D(bug("Zoomgad:  %p\n", SYSGAD(w, ZOOMGAD ) ));
	D(bug("Closegad: %p\n", SYSGAD(w, CLOSEGAD) ));
	D(bug("Sizegad:  %p\n", SYSGAD(w, SIZEGAD ) ));
	    
	/* Don't need drawinfo anymore */
	FreeScreenDrawInfo(w->WScreen, dri);
	
	if (sysgads_ok)
	{
	    UWORD i;
	
	
	    D(bug("Adding gadgets\n"));
	    for (i = 0; i < NUM_SYSGADS; i ++)
	    {
		if (SYSGAD(w, i))
		    AddGList(w, (struct Gadget *)SYSGAD(w, i), 0, 1, NULL);
	    }
	    
	    D(bug("Refreshing frame\n"));


	    ReturnBool("createsysgads", TRUE);
	    
	} /* if (sysgads created) */
	
	disposesysgads(w, IntuitionBase);
	
    } /* if (got DrawInfo) */
    ReturnBool("createsysgads", FALSE);

}


static VOID disposesysgads(struct Window *w, struct IntuitionBase *IntuitionBase)
{
    /* Free system gadges */
    UWORD i;
    
    for (i = 0; i < NUM_SYSGADS; i ++)
    {
        if (SYSGAD(w, i))
	    DisposeObject( SYSGAD(w, i) );
    }

}

void intui_ScrollWindowRaster(struct Window * win,
                              WORD dx,
                              WORD dy,
                              WORD xmin,
                              WORD ymin,
                              WORD xmax,
                              WORD ymax,
                              struct IntuitionBase * IntuitionBase)
{
  ScrollRasterBF(win->RPort,
                 dx,
                 dy,
                 xmin,
                 ymin,
                 xmax,
                 ymax);
  /* Has there been damage to the layer? */
  if (0 != (win->RPort->Layer->Flags & LAYERREFRESH))
  {
    /* 
       Send a refresh message to the window if it doesn't already
       have one.
    */
    windowneedsrefresh(win, IntuitionBase);
  } 
}

void windowneedsrefresh(struct Window * w, 
                        struct IntuitionBase * IntuitionBase )
{
  /* Supposed to send a message to this window, saying that it needs a
     refresh. I will check whether there is no such a message queued in
     its messageport, though. It only needs one such message! 
  */
  struct IntuiMessage * IM;
  BOOL found = FALSE;

  if (NULL == w->UserPort)
    return;
  
  /* Can use Forbid() for this */
  
  Forbid();
  
  IM = (struct IntuiMessage *)w->UserPort->mp_MsgList.lh_Head;

  /* reset the flag in the layer */
  w->WLayer->Flags &= ~LAYERREFRESH;

  while ((NULL != IM) && !found)
  {
    /* Does the window already have such a message? */
    if (IDCMP_REFRESHWINDOW == IM->Class)
    {
kprintf("Window %s already has a refresh message pending!!\n",w->Title);
      found = TRUE;
    }
    IM = (struct IntuiMessage *)IM->ExecMessage.mn_Node.ln_Succ;
  }
  
  Permit();

kprintf("Sending a refresh message to window %s!!\n",w->Title);
  if (!found)
  {
    IM = alloc_intuimessage(IntuitionBase);
    if (NULL != IM)
    {
      IM->Class = IDCMP_REFRESHWINDOW;
      send_intuimessage(IM, w, IntuitionBase);
    }
  }
}
