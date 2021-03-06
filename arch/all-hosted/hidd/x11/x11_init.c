/*
    Copyright � 1995-2009, The AROS Development Team. All rights reserved.
    $Id$

    Desc: X11 hidd initialization code.
    Lang: English.
*/

#define __OOP_NOATTRBASES__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>

#include <proto/exec.h>
#include <proto/oop.h>

#include <utility/utility.h>
#include <oop/oop.h>
#include <hidd/graphics.h>

#include <aros/symbolsets.h>

#undef  SDEBUG
#undef  DEBUG
#define DEBUG 0
#include <aros/debug.h>

#include LC_LIBDEFS_FILE

#include "x11.h"
#include "fullscreen.h"

/****************************************************************************************/

#undef XSD

/****************************************************************************************/

static BOOL initclasses( struct x11_staticdata *xsd );
static VOID freeclasses( struct x11_staticdata *xsd );
struct Task *create_x11task( struct x11task_params *params);
VOID x11task_entry(struct x11task_params *xtp);

/****************************************************************************************/

static OOP_AttrBase HiddPixFmtAttrBase;

static struct OOP_ABDescr abd[] =
{
    { IID_Hidd_PixFmt   , &HiddPixFmtAttrBase   },
    { NULL     	    	, NULL	    	    	}
};

/****************************************************************************************/

static BOOL initclasses(struct x11_staticdata *xsd)
{
    /* Get some attrbases */
    
    if (!OOP_ObtainAttrBases(abd))
    	goto failure;

    return TRUE;
        
failure:
    freeclasses(xsd);

    return FALSE; 
}

/****************************************************************************************/

static VOID freeclasses(struct x11_staticdata *xsd)
{
    OOP_ReleaseAttrBases(abd);
}

/****************************************************************************************/

static int MyErrorHandler (Display * display, XErrorEvent * errevent)
{
    char buffer[256];

    XCALL(XGetErrorText, display, errevent->error_code, buffer, sizeof (buffer));

    kprintf("XError %d (Major=%d, Minor=%d) task = %s\n%s\n",
	    errevent->error_code,
	    errevent->request_code,
	    errevent->minor_code,
	    FindTask(0)->tc_Node.ln_Name,
	    buffer);
	     
    return 0;
}

/****************************************************************************************/

static int MySysErrorHandler (Display * display)
{
    perror ("X11-Error");

    // *((ULONG *)0) = 0;

    return 0;
}

/****************************************************************************************/

static int X11_Init(LIBBASETYPEPTR LIBBASE)
{
    struct x11_staticdata *xsd = &LIBBASE->xsd;

    D(bug("Entering X11_Init\n"));
    if (LIBBASE->library.lib_OpenCnt) {
	D(bug("[X11GFX] Already initialized\n"));
	return TRUE;
    }
    
    InitSemaphore( &xsd->sema );
    InitSemaphore( &xsd->x11sema );
		
    /* Do not need to singlethead this
     * since no other tasks are using X currently
     */

    xsd->display = XCALL(XOpenDisplay, NULL);
    if (xsd->display)
    {
	struct x11task_params 	 xtp;
	struct Task 	    	*x11task;

	XCALL(XSetErrorHandler, MyErrorHandler);
	XCALL(XSetIOErrorHandler, MySysErrorHandler);

        /*
         * XXX on my system, getenv() is declared:
         *
         * extern char *getenv (__const char *__name) __THROW __nonnull ((1)) * __wur;
         *
         * the attributes appear to change the calling convention, so a naive
         * prototype like char *getenv(char *) causes carshes as the returned
         * address is not a valid pointer.
         *
         * ideally this configration variable would be brought in via a
         * bootloader.resource, which hosted doesn't have yet
         */
        /*
	if (getenv, "AROS_X11_FULLSCREEN")
	{
	    xsd->fullscreen = x11_fullscreen_supported(xsd->display);
	}
        */
	
	xsd->delete_win_atom         = XCALL(XInternAtom, xsd->display, "WM_DELETE_WINDOW", FALSE);
	xsd->clipboard_atom          = XCALL(XInternAtom, xsd->display, "CLIPBOARD", FALSE);
	xsd->clipboard_property_atom = XCALL(XInternAtom, xsd->display, "AROS_HOSTCLIP", FALSE);
	xsd->clipboard_incr_atom     = XCALL(XInternAtom, xsd->display, "INCR", FALSE);
	xsd->clipboard_targets_atom  = XCALL(XInternAtom, xsd->display, "TARGETS", FALSE);
	
	xtp.parent = FindTask(NULL);
	xtp.ok_signal	= SIGBREAKF_CTRL_E;
	xtp.fail_signal	= SIGBREAKF_CTRL_F;
	xtp.kill_signal	= SIGBREAKF_CTRL_C;
	xtp.xsd		= xsd;

	if ((x11task = create_x11task(&xtp)))
	{			
	    if (initclasses(xsd))
	    {
		D(bug("X11_Init succeeded\n"));
		return TRUE;
	    }
	    
	    Signal(x11task, xtp.kill_signal);
	}

	XCALL(XCloseDisplay, xsd->display);

    }
    
    D(bug("X11_Init failed\n"));
    
    return FALSE;
}

/****************************************************************************************/

ADD2OPENLIB(X11_Init, 0);
