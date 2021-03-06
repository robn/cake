/*
    Copyright � 1995-2009, The AROS Development Team. All rights reserved.
    $Id$

    Desc: X11 hidd. Connects to the X server and receives events.
    Lang: English.
*/


#include <proto/exec.h>
#include <proto/oop.h>
#include <proto/utility.h>

#define size_t aros_size_t
#include <hidd/unixio.h>
#include <hidd/hidd.h>

#include <oop/ifmeta.h>

#include <dos/dos.h>

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/resident.h>
#include <hardware/intbits.h>
#include <utility/utility.h>

#include <aros/asmcall.h>
#undef size_t

#define timeval sys_timevalinit_x11class
#include <sys/types.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#undef timeval

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include "x11.h"
#include "fullscreen.h"
#include "x11gfx_intern.h"

#define DEBUG 0
#include <aros/debug.h>

/****************************************************************************************/

#define NOUNIXIO    	    	1
#define BETTER_REPEAT_HANDLING  1

#define XTASK_NAME "x11hidd task"

/* We need to have highest priotity for this task, because we
are simulating an interrupt. Ie. an "interrupt handler" called
but this task should NEVER be interrupted by a task (for example input.device),
otherwize it will give strange effects, especially in the circular-buffer handling
in gameport/keyboard. (Writing to the buffer is not atomic even
from within the IRQ handler!)

 Instead of calling
the irq handler directly from the task, we should instead 
Cause() a software irq, but Cause() does not work at the moment..
*/

#define XTASK_PRIORITY  50

#define XTASK_STACKSIZE (AROS_STACKSIZE)

#undef XSD
#define XSD(cl) xsd

/****************************************************************************************/

#if NOUNIXIO

/****************************************************************************************/

AROS_UFH4(ULONG, x11VBlank,
    AROS_UFHA(ULONG, dummy, A0),
    AROS_UFHA(void *, data, A1),
    AROS_UFHA(ULONG, dummy2, A5),
    AROS_UFHA(struct ExecBase *, SysBase, A6))
{
    AROS_USERFUNC_INIT

    Signal((struct Task *)data, SIGBREAKF_CTRL_D);
    
    return 0;

    AROS_USERFUNC_EXIT
}

/****************************************************************************************/

#else

/****************************************************************************************/

static int unixio_callback(int displayfd, struct x11_staticdata *xsd)
{
    int pending;
    
    LOCK_X11    
    pending = XCALL(XPending, xsd->display);
    UNLOCK_X11

    return pending;
}

/****************************************************************************************/

#endif

/****************************************************************************************/

VOID x11task_entry(struct x11task_params *xtpparam)
{
    struct x11_staticdata   *xsd;
    struct MinList  	     nmsg_list;
    struct MinList  	     xwindowlist;    
    struct x11task_params    xtp;
    ULONG   	    	     hostclipboardmask;
    BOOL    	    	     f12_down = FALSE;
    KeySym  	    	     ks;
    
#if NOUNIXIO
    struct Interrupt 	     myint;
#else
    struct MsgPort  	    *unixio_port = NULL;
    HIDD    	    	    *unixio = NULL;
    IPTR    	    	     ret;
    ULONG   	    	     unixiosig;
    BOOL    	    	     domouse = FALSE;
    LONG    	    	     last_mouse_x;
    LONG    	    	     last_mouse_y;    
    BOOL    	    	     dounixio = TRUE;    
#endif

    /* We must copy the parameter struct because they are allocated
     on the parent's stack */
    xtp = *xtpparam;
    xsd = xtp.xsd;


    xsd->x11task_notify_port = CreateMsgPort();
    if (NULL == xsd->x11task_notify_port)
    	goto failexit;
    	
    NEWLIST(&nmsg_list);
    NEWLIST(&xwindowlist);
  
#if NOUNIXIO
    
    myint.is_Code         = (APTR)&x11VBlank;
    myint.is_Data         = FindTask(NULL);
    myint.is_Node.ln_Name = "X11 VBlank server";
    myint.is_Node.ln_Pri  = 0;
    myint.is_Node.ln_Type = NT_INTERRUPT;
	
    AddIntServer(INTB_TIMERTICK, &myint);


    Signal(xtp.parent, xtp.ok_signal);

#else
    
    unixio = (HIDD)New_UnixIO(OOPBase, SysBase);
    if (unixio)
    {
    	unixio_port = CreateMsgPort();
	if (unixio_port)
	{
	    unixiosig = 1L << unixio_port->mp_SigBit;
	    Signal(xtp.parent, xtp.ok_signal);
	     
	}
	else goto failexit;
    }
    else goto failexit;
#endif    

    hostclipboardmask = x11clipboard_init(xsd);
    
    for (;;)
    {
	XEvent      	     event;	
    #if BETTER_REPEAT_HANDLING
	XEvent  	     keyrelease_event;
	BOOL    	     keyrelease_pending = FALSE;
    #endif        
	struct notify_msg   *nmsg;	
	ULONG 	    	     notifysig = 1L << xsd->x11task_notify_port->mp_SigBit;
	ULONG 	    	     sigs;

#if NOUNIXIO

	sigs = Wait(SIGBREAKF_CTRL_D | notifysig | xtp.kill_signal | hostclipboardmask);
	
#else	


    #if 0


    	ret = (int)Hidd_UnixIO_Wait(unixio,
	    	    	    	    ConnectionNumber( xsd->display ),
				    vHidd_UnixIO_Read,
				    unixio_callback,
				    (APTR)xsd,
				    xtp.kill_signal | notifysig | hostclipboardmask);
			
			
    #else

	if (dounixio)
	{
	    ret = Hidd_UnixIO_AsyncIO(unixio,
	     	    	    	       ConnectionNumber(xsd->display),  vHidd_UnixIO_Terminal,
				       unixio_port, vHidd_UnixIO_Read, SysBase);
	
	    if (ret)
	    {
	    
	    	kprintf("ERROR WHEN CALLING UNIXIO: %d\n", ret);
		dounixio = TRUE;
		
	        continue;
	    }
	    else
	    {
	    	dounixio = FALSE;
	    }
	}
	
	sigs = Wait(notifysig | unixiosig | xtp.kill_signal | hostclipboardmask);			
D(bug("Got input from unixio\n"));
/*			
	if (ret != 0)
	{
	    continue;
	}
	
	
*/
	
	if (sigs & unixiosig)
	{
	     struct uioMessage *uiomsg;
	     int    	    	result;
	     
	     uiomsg = (struct uioMessage *)GetMsg(unixio_port);
	     result = uiomsg->result;
	     
	     FreeMem(uiomsg, sizeof (struct uioMessage));
	     
	     dounixio = TRUE;
	     
	     if (result)
	     	continue;
	}
	
    #endif


#endif
	if (sigs & xtp.kill_signal)
	    goto failexit;
	
	if (sigs & notifysig)
	{

	    while ((nmsg = (struct notify_msg *)GetMsg(xsd->x11task_notify_port)))
	    {
		/* Add the messages to an internal list */
		
		switch (nmsg->notify_type)
		{		
		    case NOTY_WINCREATE:
		    {
			struct xwinnode * node;
			/* Maintain a list of open windows for the X11 event handler in x11.c */

			node = AllocMem(sizeof (struct xwinnode), MEMF_CLEAR);

			if (NULL != node)
			{

		    	    node->xwindow = nmsg->xwindow;
			    node->bmobj   = nmsg->bmobj; 
		    	    AddTail( (struct List *)&xwindowlist, (struct Node *)node );			
			}
			else
			{
		    	    kprintf("!!!! CANNOT GET MEMORY FOR X11 WIN NODE\n");
		    	    CCALL(raise, 19);
			}

			ReplyMsg((struct Message *)nmsg);
			break;
		    }
		
		    case NOTY_MAPWINDOW:
    	    		LOCK_X11		
	        	XCALL(XMapWindow, nmsg->xdisplay, nmsg->xwindow);
    	    	    #if ADJUST_XWIN_SIZE
			XCALL(XMapRaised, nmsg->xdisplay, nmsg->masterxwindow);
    	    	    #endif
    	    		UNLOCK_X11

			AddTail((struct List *)&nmsg_list, (struct Node *)nmsg);			

			/* Do not reply message yet */
			break;
		    
		    case NOTY_RESIZEWINDOW:
		    {
			XWindowChanges xwc;
    	    	    	BOOL replymsg = TRUE;
			struct xwinnode *node;
			
			xwc.width  = nmsg->width;
			xwc.height = nmsg->height;

    	    		LOCK_X11
			if (xsd->fullscreen)
			{
			    x11_fullscreen_switchmode(nmsg->xdisplay, &xwc.width, &xwc.height);   
			}	
			XCALL(XConfigureWindow, nmsg->xdisplay
		    	    , nmsg->masterxwindow
		    	    , CWWidth | CWHeight
			    , &xwc
			);
			XCALL(XFlush, nmsg->xdisplay);
    	    		UNLOCK_X11
			
    	    	    #if DELAY_XWIN_MAPPING	
		    	ForeachNode(&xwindowlist, node)
			{
		    	    if (node->xwindow == nmsg->xwindow)
			    {			
				if (!node->window_mapped)
				{
		    		    LOCK_X11
				    XCALL(XMapWindow, nmsg->xdisplay, nmsg->xwindow);
	    			#if ADJUST_XWIN_SIZE
		    		    XCALL(XMapRaised, nmsg->xdisplay, nmsg->masterxwindow);
				#endif
				    if (xsd->fullscreen)
				    {
    	    	    	    	    	XCALL(XGrabKeyboard, nmsg->xdisplay,nmsg->xwindow,False,GrabModeAsync,GrabModeAsync,CurrentTime);
    	    	    	    	    	XCALL(XGrabPointer, nmsg->xdisplay,nmsg->xwindow, 1, PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
		    	    	    	    	      GrabModeAsync, GrabModeAsync, nmsg->xwindow, None, CurrentTime);
				    }
				    XCALL(XFlush, nmsg->xdisplay);		
				    UNLOCK_X11

    	    	    	    	    nmsg->notify_type = NOTY_MAPWINDOW;
				    AddTail((struct List *)&nmsg_list, (struct Node *)nmsg);
				    
				    /* Do not reply message yet */
				    replymsg = FALSE;
				    
				    break;
				}
			    }
			}	    	
		    #endif
		    
			if (replymsg) ReplyMsg((struct Message *)nmsg);		    
			
			break;
		    }
		
		    case NOTY_WINDISPOSE:
		    {
			struct xwinnode *node, *safe;


			ForeachNodeSafe(&xwindowlist, node, safe)
			{
		    	    if (node->xwindow == nmsg->xwindow)
			    {
				 Remove((struct Node *)node);				
				 FreeMem(node, sizeof (struct xwinnode));			     
			    }
			}

			ReplyMsg((struct Message *)nmsg);

			break;
		    }
		    
		} /* switch() */
		
	    } /* while () */
	    
	    //continue;
	    
	} /* if (message from notify port) */

    	if (sigs & hostclipboardmask)
	{
	    x11clipboard_handle_commands(xsd);
	}
	
 	for (;;)	    
	{
	    struct xwinnode *node;
	    int     	     pending;
	    BOOL    	     window_found = FALSE;

    	    LOCK_X11
	    XCALL(XFlush, xsd->display);
	    XCALL(XSync, xsd->display, FALSE);
	    pending = XCALL(XEventsQueued, xsd->display, QueuedAlready);
    	    UNLOCK_X11
	    
	    if (pending == 0)
	    {
	    #if BETTER_REPEAT_HANDLING
	    	if (keyrelease_pending)
		{
		    if (XCALL(XLookupKeysym, (XKeyEvent *)&keyrelease_event, 0) == XK_F12)
		    {
			f12_down = FALSE;
		    }

	    	    ObtainSemaphoreShared( &xsd->sema );
		    if (xsd->kbdhidd)
		    {
			Hidd_X11Kbd_HandleEvent(xsd->kbdhidd, &keyrelease_event);
		    }
		    ReleaseSemaphore( &xsd->sema );
		    keyrelease_pending = FALSE;
		}		
	    #endif
	    
	    	/* Get out of for(;;) loop */
		break;
	    }
	    
    	    LOCK_X11
	    XCALL(XNextEvent, xsd->display, &event);
    	    UNLOCK_X11

	    D(bug("Got Event for X=%d\n", event.xany.window));

    	#if BETTER_REPEAT_HANDLING
	    if (keyrelease_pending)
	    {
	    	BOOL repeated_key = FALSE;
		
		/* Saw this in SDL/X11 code, where a comment says that
		   the idea for this was coming from GII, whatever that
		   is. */
		   
		if ((event.xany.window == keyrelease_event.xany.window) &&
		    (event.type == KeyPress) &&
		    (event.xkey.keycode == keyrelease_event.xkey.keycode) &&
		    ((event.xkey.time - keyrelease_event.xkey.time) < 2))
		{
		    repeated_key = TRUE;
		}
		
		keyrelease_pending = FALSE;
		
		if (repeated_key)
		{
		    /* Drop both previous keyrelease and this keypress event. */
		    continue;
		}
		
		LOCK_X11
		if (XCALL(XLookupKeysym, (XKeyEvent *)&keyrelease_event, 0) == XK_F12)
		{
		    f12_down = FALSE;
		}
    	    	UNLOCK_X11
		
	    	ObtainSemaphoreShared( &xsd->sema );
		if (xsd->kbdhidd)
		{
		    Hidd_X11Kbd_HandleEvent(xsd->kbdhidd, &keyrelease_event);
		}
		ReleaseSemaphore( &xsd->sema );		
	    }	    
	#endif
	
	    if (event.type == MappingNotify)
	    {
    	    	LOCK_X11
		XCALL(XRefreshKeyboardMapping, (XMappingEvent*)&event);
    	    	UNLOCK_X11

		continue;
	    }
	    
    	#if ADJUST_XWIN_SIZE
	    /* Must check this here, because below only the inner
	       window events are recognized */
	       
	    if ((event.type == ClientMessage) &&
	        (event.xclient.data.l[0] == xsd->delete_win_atom))
	    {
		CCALL(raise, SIGINT);
	    }
    	#endif	    

	    ForeachNode( &xwindowlist, node)
	    {
	        if (node->xwindow == event.xany.window)
		{
		    window_found = TRUE;
		    break;
		}
	    }
	    
	    if (x11clipboard_want_event(&event))
	    {
	    	x11clipboard_handle_event(xsd, &event);
	    }
	    	    
	    if (window_found)
	    {
	        D(bug("Got event for window %x\n", event.xany.window));
	    	switch (event.type)
	    	{
	    	    case GraphicsExpose:
	    	    case Expose:
			break;

		    case ConfigureRequest:
			kprintf("!!! CONFIGURE REQUEST !!\n");
			break;

    		#if 0
    	    	    /* stegerg: not needed */
	            case ConfigureNotify:
		    {
			/* The window has been resized */

			XConfigureEvent     *me;
			struct notify_msg   *nmsg, *safe;

			me = (XConfigureEvent *)&event;
			ForeachNodeSafe(&nmsg_list, nmsg, safe)
			{
		    	    if (    me->window == nmsg->xwindow
				 && nmsg->notify_type == NOTY_RESIZEWINDOW)
			    {
				 /*  The window has now been mapped.
			             Send reply to app */

				 Remove((struct Node *)nmsg);
				 ReplyMsg((struct Message *)nmsg);
			    }
			}



			break;
		    }
    		#endif

	    	    case ButtonPress:
		    	xsd->x_time = event.xbutton.time;
			D(bug("ButtonPress event\n"));

	    		ObtainSemaphoreShared( &xsd->sema );
			if (xsd->mousehidd)
			    Hidd_X11Mouse_HandleEvent(xsd->mousehidd, &event);
			ReleaseSemaphore( &xsd->sema );
			break;

	    	    case ButtonRelease:
		    	xsd->x_time = event.xbutton.time;
			D(bug("ButtonRelease event\n"));

	    		ObtainSemaphoreShared( &xsd->sema );
			if (xsd->mousehidd)
			    Hidd_X11Mouse_HandleEvent(xsd->mousehidd, &event);
			ReleaseSemaphore( &xsd->sema );
			break;
			
	    	    case MotionNotify:
		    	xsd->x_time = event.xmotion.time;
			D(bug("Motionnotify event\n"));

	    		ObtainSemaphoreShared( &xsd->sema );
			if (xsd->mousehidd)
			    Hidd_X11Mouse_HandleEvent(xsd->mousehidd, &event);
			ReleaseSemaphore( &xsd->sema );
			break;

		    case FocusOut:
		    #if !BETTER_REPEAT_HANDLING
    	    	    	LOCK_X11
			XCALL(XAutoRepeatOn, xsd->display);
    	    	    	UNLOCK_X11
		    #endif
		    
    	    	    #if 0
    	    	    	ObtainSemaphoreShared(&xsd->sema);
			/* Call the user supplied callback func, if supplied */
			if (NULL != xsd->activecallback)
			{
		    	    xsd->activecallback(xsd->callbackdata, node->bmobj, FALSE);
			}
    	    	    	ReleaseSemaphore(&xsd->sema);
    	    	    #endif
			break;

		    case FocusIn:
    	    	    #if 0		
    	    	    	ObtainSemaphoreShared(&xsd->sema);
			/* Call the user supplied callback func, if supplied */
			if (NULL != xsd->activecallback)
			{
		    	    xsd->activecallback(xsd->callbackdata, node->bmobj, TRUE);
			}
    	    	    	ReleaseSemaphore(&xsd->sema);
    	    	    #endif
			break;

	    	    case KeyPress:
		    	xsd->x_time = event.xkey.time;
			
    	    	    	LOCK_X11
		    #if !BETTER_REPEAT_HANDLING
    			XCALL(XAutoRepeatOff, XSD(cl)->display);
		    #endif		    
		    	ks = XCALL(XLookupKeysym, (XKeyEvent *)&event, 0);
			if (ks == XK_F12)
			{
			    f12_down = TRUE;
			}
			else if (f12_down && ((ks == XK_Q) || (ks == XK_q)))
			{
			    CCALL(raise, SIGINT);
			}
    	    	    	UNLOCK_X11	

	    		ObtainSemaphoreShared( &xsd->sema );
			if (xsd->kbdhidd)
			{
			    Hidd_X11Kbd_HandleEvent(xsd->kbdhidd, &event);
			}
			ReleaseSemaphore( &xsd->sema );
			break;


	    	    case KeyRelease:
		    	xsd->x_time = event.xkey.time;
						
		    #if BETTER_REPEAT_HANDLING
		    	keyrelease_pending = TRUE;
			keyrelease_event = event;
		    #else		    
    	    	    	LOCK_X11
		    	if (XCALL(XLookupKeysym, &event, 0) == XK_F12)
			{
			    f12_down = FALSE;
			}
			XCALL(XAutoRepeatOn, XSD(cl)->display);
    	    	    	UNLOCK_X11

	    		ObtainSemaphoreShared( &xsd->sema );
			if (xsd->kbdhidd)
			{
			    Hidd_X11Kbd_HandleEvent(xsd->kbdhidd, &event);
			}
			ReleaseSemaphore( &xsd->sema );
		    #endif
			break;

	   	    case EnterNotify:
			break;

	    	    case LeaveNotify:
			break;

		    case MapNotify:
		    {

			XMapEvent   	    *me;
			struct notify_msg   *nmsg, *safe;
			struct xwinnode     *node;
			BOOL 	    	     found = FALSE;

			me = (XMapEvent *)&event;

			ForeachNodeSafe(&nmsg_list, nmsg, safe)
			{
		    	    if (me->window == nmsg->xwindow
				&& nmsg->notify_type == NOTY_MAPWINDOW)
			    {
				 /*  The window has now been mapped.
			             Send reply to app */

		 		 found = TRUE;
				 Remove((struct Node *)nmsg);
				 ReplyMsg((struct Message *)nmsg);
			    }
			}

			/* Find it in thw window list and mark it as mapped */

			ForeachNode(&xwindowlist, node)
			{
		    	    if (node->xwindow == me->window)
			    {			
				node->window_mapped = TRUE;			
			    }
			}


			break;
		    }

    	    	#if !ADJUST_XWIN_SIZE
         	    case ClientMessage:
            		if (event.xclient.data.l[0] == xsd->delete_win_atom)
			{
		            CCALL(raise, SIGINT);
			}
			break;
    	    	#endif
			
	        } /* switch (X11 event type) */
		
	    } /* if (is event for HIDD window) */

    	} /* while (events from X)  */
    	
    } /* Forever */
    
failexit:
    #warning "Also try to free window node list ?"

    if (xsd->x11task_notify_port)
    {
	DeleteMsgPort(xsd->x11task_notify_port);
    }
		

#if (!NOUNIXIO)
    if (NULL != unixio_port)
    {
    	DeleteMsgPort(unixio_port);
    }
	
    if (NULL != unixio)
    {
    	OOP_DisposeObject(unixio);
    }
#endif

    Signal(xtp.parent, xtp.fail_signal);
    
}

/****************************************************************************************/

struct Task *create_x11task( struct x11task_params *params)
{
    struct Task *task;
    APTR    	 stack;
    
    task = AllocMem(sizeof (struct Task), MEMF_PUBLIC|MEMF_CLEAR);
    if (task)
    {
    	NEWLIST(&task->tc_MemEntry);
    	task->tc_Node.ln_Type =NT_TASK;
    	task->tc_Node.ln_Name = XTASK_NAME;
    	task->tc_Node.ln_Pri  = XTASK_PRIORITY;

    	stack = AllocMem(XTASK_STACKSIZE, MEMF_PUBLIC);
    	if(stack != NULL)
    	{
	    struct TagItem tags[] =
	    {
	    	 {TASKTAG_ARG1, (IPTR)params},
		 {TAG_DONE  	    	    }
	    };
	    
	    task->tc_SPLower = stack;
	    task->tc_SPUpper = (UBYTE *)stack + XTASK_STACKSIZE;

    	#if AROS_STACK_GROWS_DOWNWARDS
	    task->tc_SPReg = (UBYTE *)task->tc_SPUpper-SP_OFFSET;
    	#else
	    task->tc_SPReg = (UBYTE *)task->tc_SPLower+SP_OFFSET;
    	#endif
	    
	    /* You have to clear signals first. */
	    SetSignal(0, params->ok_signal | params->fail_signal);

	    if(NewAddTask(task, x11task_entry, NULL, tags) != NULL)
	    {
	    	/* Everything went OK. Wait for task to initialize */
		ULONG sigset;
		

		sigset = Wait( params->ok_signal | params->fail_signal );
		if (sigset & params->ok_signal)
		{
		    return task;
		}
		
	    }	
	    FreeMem(stack, XTASK_STACKSIZE);
	    
    	}
        FreeMem(task,sizeof(struct Task));
	
    }
    return NULL;
}

/****************************************************************************************/
