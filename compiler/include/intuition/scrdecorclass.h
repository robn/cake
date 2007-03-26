#ifndef INTUITION_SCRDECORCLASS_H
#define INTUITION_SCRDECORCLASS_H

/*
    Copyright  1995-2001, The AROS Development Team. All rights reserved.
    $Id: screendecorclass.h 12757 2001-12-08 22:23:57Z dariusb $

    Desc: Headerfile for Intuitions' SCRDECORCLASS
    Lang: english
*/

#ifndef UTILITY_TAGITEM_H
#   include <utility/tagitem.h>
#endif

#ifndef INTUITION_IMAGECLASS_H
#   include <intuition/imageclass.h>
#endif

#ifndef GRAPHICS_CLIP_H
#   include <graphics/clip.h>
#endif

#ifndef INTUITION_INTUITION_H
#   include <intuition/intuition.h>
#endif

#ifndef INTUITION_SCREENS_H
#   include <intuition/screens.h>
#endif

/* Attributes for SCRDECORCLASS */
#define SDA_Dummy		    (TAG_USER + 0x22100)
#define SDA_DrawInfo	    	    (SDA_Dummy + 1) 	    /* I.G */
#define SDA_Screen  	    	    (SDA_Dummy + 2) 	    /* I.G */
#define SDA_TrueColorOnly	    (SDA_Dummy + 3) 	    /* ..G */
#define SDA_UserBuffer              (SDA_Dummy + 4)         /* I.G */

/* Methods for SCRDECORCLASS */
#define SDM_Dummy   	    	    (SDA_Dummy + 500)

#define SDM_SETUP   	    	    (SDM_Dummy + 1)
#define SDM_CLEANUP 	    	    (SDM_Dummy + 2)
#define SDM_GETDEFSIZE_SYSIMAGE     (SDM_Dummy + 3)
#define SDM_DRAW_SYSIMAGE   	    (SDM_Dummy + 4)
#define SDM_DRAW_SCREENBAR  	    (SDM_Dummy + 5)
#define SDM_LAYOUT_SCREENGADGETS    (SDM_Dummy + 6)
#define SDM_INITSCREEN              (SDM_Dummy + 7)
#define SDM_EXITSCREEN              (SDM_Dummy + 8)

struct sdpGetDefSizeSysImage
{
    STACKULONG	     MethodID;
    BOOL             sdp_TrueColor;
    struct DrawInfo *sdp_Dri;
    struct TextFont *sdp_ReferenceFont; /* In: */
    STACKULONG	     sdp_Which;  	/* In: SDEPTHIMAGE */
    STACKULONG	     sdp_SysiSize;	/* In: lowres/medres/highres */
    STACKULONG	    *sdp_Width;  	/* Out */
    STACKULONG	    *sdp_Height; 	/* Out */
    STACKULONG	     sdp_Flags;
    STACKIPTR	     sdp_UserBuffer;
};

/* This struct must match wdpDrawSysImage struct in windecorclass.h! */

struct sdpDrawSysImage
{
    STACKULONG	     MethodID;
    BOOL             sdp_TrueColor;
    struct DrawInfo *sdp_Dri;
    struct RastPort *sdp_RPort;
    STACKLONG	     sdp_X;
    STACKLONG	     sdp_Y;
    STACKLONG	     sdp_Width;
    STACKLONG	     sdp_Height;
    STACKULONG	     sdp_Which;
    STACKULONG	     sdp_State;
    STACKULONG	     sdp_Flags;
    STACKIPTR	     sdp_UserBuffer;
};

struct sdpDrawScreenBar
{
    STACKULONG	     MethodID;
    BOOL             sdp_TrueColor;
    struct DrawInfo *sdp_Dri;
    struct Layer    *sdp_Layer;
    struct RastPort *sdp_RPort;
    struct Screen   *sdp_Screen;
    STACKULONG	     sdp_Flags;
    STACKIPTR	     sdp_UserBuffer;
};

struct sdpLayoutScreenGadgets
{
    STACKULONG	     MethodID;
    BOOL             sdp_TrueColor;
    struct DrawInfo *sdp_Dri;
    struct Layer    *sdp_Layer;
    struct Gadget   *sdp_Gadgets;
    STACKULONG	     sdp_Flags;
    STACKIPTR	     sdp_UserBuffer;
};

struct sdpInitScreen
{
    STACKULONG	     MethodID;
    BOOL             sdp_TrueColor;
    struct DrawInfo *sdp_Dri;
    STACKULONG       sdp_FontHeight;
    STACKLONG        sdp_TitleHack;
    STACKULONG       sdp_BarHeight;
    STACKULONG       sdp_BarVBorder;
    STACKULONG       sdp_BarHBorder;
    STACKULONG       sdp_MenuVBorder;
    STACKULONG       spd_MenuHBorder;
    STACKBYTE        sdp_WBorTop;
    STACKBYTE        sdp_WBorLeft;
    STACKBYTE        sdp_WBorRight;
    STACKBYTE        sdp_WBorBottom;
    STACKIPTR        sdp_UserBuffer;
};

struct sdpExitScreen
{
    STACKULONG       MethodID;
    BOOL             sdp_TrueColor;
    STACKIPTR	     sdp_UserBuffer;
};
/* ScrDecor LayoutScreenGadgets Flags */
#define SDF_LSG_INITIAL     	1   /* First time == During OpenScreen */
#define SDF_LSG_SYSTEMGADGET	2   /* Is a system gadget (sdepth) */
#define SDF_LSG_INGADLIST   	4   /* Gadget is already in screen gadget list */
#define SDF_LSG_MULTIPLE    	8   /* There may be multiple gadgets (linked
                                       together through NextGadget. Follow it) */
				       
#endif /* INTUITION_SCRDECORCLASS_H */
