#ifndef HIDD_GDI_H
#define HIDD_GDI_H

/*
    Copyright  1995-2008, The AROS Development Team. All rights reserved.
    $Id: gdi.h 27106 2007-10-28 10:49:03Z verhaegs $

    Desc: Include for the gdi HIDD.
    Lang: English.
*/

#ifndef WM_USER
#define WM_USER 1024
#endif

#ifdef __AROS__

#include <exec/libraries.h>
#include <oop/oop.h>
#include <exec/semaphores.h>

/* #define GDI_LOAD_KEYMAPTABLE	1*/

#include "winapi.h"
#include "gdi_hostlib.h"

/***** GDIMouse HIDD *******************/

/* Private data */
struct pHidd_Mouse_Event;
struct gdimouse_data
{
    VOID (*mouse_callback)(APTR, struct pHidd_Mouse_Event *);
    APTR callbackdata;
    void *interrupt;
    UWORD buttons;
};

/* IDs */
#define IID_Hidd_GDIMouse	"hidd.mouse.gdi"
#define CLID_Hidd_GDIMouse	"hidd.mouse.gdi"

/***** GDIKbd HIDD *******************/

/* Private data */
struct gdikbd_data
{
    VOID  (*kbd_callback)(APTR, UWORD);
    APTR    callbackdata;
    void *interrupt;
};

/* IDs */
#define IID_Hidd_GDIKbd		"hidd.kbd.gdi"
#define CLID_Hidd_GDIKbd	"hidd.kbd.gdi"

struct gdi_staticdata
{
    struct SignalSemaphore   sema; /* Protecting this whole struct */
     
    APTR 	    	     display;
    
    OOP_Class 	    	    *gfxclass;
    OOP_Class 	    	    *bmclass;
    OOP_Class 	    	    *mouseclass;
    OOP_Class 	    	    *kbdclass;
    
    OOP_Object      	    *gfxhidd;
    OOP_Object      	    *mousehidd;
    OOP_Object      	    *kbdhidd;

    ULONG		     red_mask;
    ULONG		     green_mask;
    ULONG		     blue_mask;
    ULONG   	    	     red_shift;
    ULONG   	    	     green_shift;
    ULONG   	    	     blue_shift;
    ULONG   	    	     depth; /* Size of pixel in bits */

    ULONG		     window_ready;

/*  ULONG   	    	     bytes_per_pixel;
    ULONG   	    	     clut_shift;
    ULONG   	    	     clut_mask;

    Atom    	    	     delete_win_atom;
    Atom    	    	     clipboard_atom;
    Atom    	    	     clipboard_property_atom;
    Atom    	    	     clipboard_incr_atom;
    Atom    	    	     clipboard_targets_atom;

    Time    	    	     x_time;
#if 0
    VOID	    	     (*activecallback)(APTR, OOP_Object *, BOOL);
    APTR	    	     callbackdata;
#endif    

    BOOL    	    	    fullscreen;

    struct MsgPort  	    *hostclipboardmp;
    struct Message  	    *hostclipboardmsg;
    ULONG   	    	     hostclipboard_readstate;
    unsigned char   	    *hostclipboard_incrbuffer;
    ULONG   	    	     hostclipboard_incrbuffer_size;
    unsigned char   	    *hostclipboard_writebuffer;
    ULONG   	    	     hostclipboard_writebuffer_size;
    Window    	    	     hostclipboard_writerequest_window;
    Atom    	    	     hostclipboard_writerequest_property;
    ULONG   	    	     hostclipboard_write_chunks;*/
};

struct gdiclbase
{
    struct Library        library;
    
    struct gdi_staticdata xsd;
};

#define HOSTCLIPBOARDSTATE_IDLE     	0
#define HOSTCLIPBOARDSTATE_READ     	1
#define HOSTCLIPBOARDSTATE_READ_INCR    2
#define HOSTCLIPBOARDSTATE_WRITE    	3
/*
VOID get_bitmap_info(struct gdi_staticdata *xsd, Drawable d, ULONG *sz, ULONG *bpl);

BOOL set_pixelformat(struct TagItem *pftags, struct gdi_staticdata *xsd, Drawable d);

ULONG gdiclipboard_init(struct gdi_staticdata *);
VOID  gdiclipboard_handle_commands(struct gdi_staticdata *);
BOOL  gdiclipboard_want_event(XEvent *);
VOID  gdiclipboard_handle_event(struct gdi_staticdata *, XEvent *);
*/
#undef XSD
#define XSD(cl)     	(&((struct gdiclbase *)cl->UserData)->xsd)

#else

#include <windows.h>
#define IPTR ULONG_PTR

#endif

#define NOTY_SHOW WM_USER

/* Private instance data for Gfx hidd class */
struct gfx_data
{
    void *display;
    ULONG depth;
/*  Cursor	 cursor;*/
    void *bitmap;    /* Currently shown bitmap object			*/
    void *fbwin;     /* Frame buffer window			        */
    void *bitmap_dc; /* Memory device context of currently shown bitmap */
    IPTR width;      /* Size of currently shown bitmap (window size)    */
    IPTR height;
};

struct MouseData
{
    unsigned short EventCode;
    unsigned short MouseX;
    unsigned short MouseY;
    unsigned short Buttons;
    unsigned short WheelDelta;
};

struct KeyboardData
{
    unsigned short EventCode;
    unsigned short KeyCode;
};

#ifdef __AROS__

void GfxIntHandler(struct gfx_data *data, struct Task *task);

#endif

#endif /* HIDD_GDI_H */
