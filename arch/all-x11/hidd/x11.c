/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc: X11 hidd. Connects to the X server and receives events.
    Lang: English.
*/

#define AROS_ALMOST_COMPATIBLE 1

#define size_t aros_size_t
#include <hidd/unixio.h>
#include <hidd/hidd.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/oop.h>
#include <proto/utility.h>

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

#define timeval sys_timeval
#include <sys/types.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#undef timeval

#include <X11/Xlib.h>
#include <X11/Xutil.h>


#include "x11.h"

#define DEBUG 0
#include <aros/debug.h>

#define NOUNIXIO 1

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

#define XTASK_PRIORITY 50

#define XTASK_STACKSIZE (AROS_STACKSIZE)


struct x11_data
{
    ULONG dummy;
    
};

static struct ABDescr attrbases[] =
{
    { NULL, NULL }
};

#define XSD(cl) ((struct x11_staticdata *)cl->UserData)
#define OOPBase		((struct Library *)XSD(cl)->oopbase)
#define UtilityBase	((struct Library *)XSD(cl)->utilitybase)

static Object *x11_new(Class *cl, Object *o, struct pRoot_New *msg)
{
    o = (Object *)DoSuperMethod(cl, o, (Msg) msg);
    if (o)
    {
    }
    return o;
}

#define IS_X11_ATTR(attr, idx) (( (idx) = (attr) - HiddX11AB) < num_Hidd_X11_Attrs)

static VOID x11_dispose(Class *cl, Object *o, Msg msg)
{
    DoSuperMethod(cl, o, (Msg)msg);
    return;
}

static VOID x11_get(Class *cl, Object *o, struct pRoot_Get *msg)
{
    return;
}


#undef XSD
#define XSD(cl) xsd
#undef SysBase



#define NUM_ROOT_METHODS 3
#define NUM_X11_METHODS 0

Class *init_x11class (struct x11_staticdata *xsd)
{
    Class *cl = NULL;

    struct MethodDescr root_descr[NUM_ROOT_METHODS + 1] = 
    {
    	{METHODDEF(x11_new),		moRoot_New},
    	{METHODDEF(x11_dispose),	moRoot_Dispose},
    	{METHODDEF(x11_get),		moRoot_Get},
	{NULL, 0UL}
    };
    
    struct MethodDescr x11hidd_descr[NUM_X11_METHODS + 1] = 
    {
	{NULL, 0UL}
    };
    
    struct InterfaceDescr ifdescr[] =
    {
    	{root_descr, 	IID_Root, 	NUM_ROOT_METHODS},
    	{x11hidd_descr, IID_Hidd_X11, 	NUM_X11_METHODS},
	{NULL, NULL, 0}
    };
    
    AttrBase MetaAttrBase = ObtainAttrBase(IID_Meta);
	
    struct TagItem tags[] =
    {
	{ aMeta_SuperID,		(IPTR)CLID_Hidd },
	{ aMeta_InterfaceDescr,		(IPTR)ifdescr},
	{ aMeta_InstSize,		(IPTR)sizeof (struct x11_data) },
	{TAG_DONE, 0UL}
    };

    EnterFunc(bug("X11HiddClass init\n"));
    
    if (MetaAttrBase)
    {
    	cl = NewObject(NULL, CLID_HiddMeta, tags);
    	if(cl)
    	{
	    cl->UserData = (APTR)xsd;
	    xsd->x11class = cl;
	    
	    if (ObtainAttrBases(attrbases))
	    {
		D(bug("X11HiddClass ok\n"));
		
	    	AddClass(cl);
	    }
	    else
	    {
	    	free_x11class(xsd);
		cl = NULL;
	    }
	}
	/* Don't need this anymore */
	ReleaseAttrBase(IID_Meta);
    }
    return cl;
}




/*************** free_x11class()  **********************************/
VOID free_x11class(struct x11_staticdata *xsd)
{
    EnterFunc(bug("free_x11class(xsd=%p)\n", xsd));

    if(xsd)
    {

        RemoveClass(xsd->x11class);
	
        if(xsd->x11class) DisposeObject((Object *) xsd->x11class);
        xsd->x11class = NULL;
	
	ReleaseAttrBases(attrbases);

    }

    ReturnVoid("free_x11class");
}

#if NOUNIXIO

AROS_UFH4(ULONG, x11VBlank,
    AROS_UFHA(ULONG, dummy, A0),
    AROS_UFHA(void *, data, A1),
    AROS_UFHA(ULONG, dummy2, A5),
    AROS_UFHA(struct ExecBase *, SysBase, A6))
{
    Signal((struct Task *)data, SIGBREAKF_CTRL_D);
    return 0;
}

#else

static int unixio_callback(int displayfd, struct x11_staticdata *xsd)
{
    int pending;
    
LX11    
    pending = XPending(xsd->display);
UX11


    return pending;
}
#endif

VOID x11task_entry(struct x11task_params *xtpparam)
{
    struct x11_staticdata *xsd;
    struct MinList nmsg_list;
    struct MinList xwindowlist;
    
    struct x11task_params xtp;
    
    
    /* We must copy the parameter struct because they are allocated
     on the parent's stack */
     


#if NOUNIXIO
    struct Interrupt myint;
#else
    struct MsgPort *unixio_port = NULL;
    HIDD *unixio = NULL;
    IPTR ret;
    ULONG unixiosig;
    
    BOOL dounixio = TRUE;
    
#endif

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
	
    AddIntServer(INTB_VERTB, &myint);


    Signal(xtp.parent, xtp.ok_signal);

#else

    
    unixio = (HIDD)New_UnixIO(OOPBase);
    if (unixio) {
    	unixio_port = CreateMsgPort();
	if (unixio_port) {
	    unixiosig = 1L << unixio_port->mp_SigBit;
	    Signal(xtp.parent, xtp.ok_signal);
	     
	} else
	    goto failexit;
    } else
    	goto failexit;
#endif    

    for (;;)
    {
	XEvent event;
	
	struct notify_msg *nmsg;
	
	
	ULONG notifysig = 1L << xsd->x11task_notify_port->mp_SigBit;
	ULONG sigs;
	
#if NOUNIXIO

	sigs = Wait(SIGBREAKF_CTRL_D | notifysig  |  xtp.kill_signal );
	

#else	


#if 0


    	ret = (int)Hidd_UnixIO_Wait( unixio
			, ConnectionNumber( xsd->display )
			, vHidd_UnixIO_Read
			, unixio_callback
			, (APTR)xsd
			, xtp.kill_signal | notifysig );
			
			
#else

	if (dounixio) {
	     ret = Hidd_UnixIO_AsyncIO(unixio
		, ConnectionNumber(xsd->display)
		, unixio_port
		, vHidd_UnixIO_Read
	     );
	
	    if (0 != ret) {
	    
	    	kprintf("ERROR WHEN CALLING UNIXIO: %d\n", ret);
		dounixio = TRUE;
		
	        continue;
	    } else {
	    	dounixio = FALSE;
	    }
	}
	
// kprintf("WAITING FOR SIGS\n");
	sigs = Wait(notifysig | unixiosig | xtp.kill_signal);			
// kprintf("GOT SIG\n");
D(bug("Got input from unixio\n"));
/*			
	if (ret != 0)
	{
	    continue;
	}
	
	
*/
	if (sigs & unixiosig) {
	     struct uioMessage *uiomsg;
	     int result;
	     
	     uiomsg = (struct uioMessage *)GetMsg(unixio_port);
	     result = uiomsg->result;
// kprintf("GOT MSG FROM UNIXIO, result = %d\n", result);
	     
	     FreeMem(uiomsg, sizeof (struct uioMessage));
	     
	     dounixio = TRUE;
	     
	     if (0 != result)
	     	continue;
	}
	
#endif


#endif

	if (sigs & xtp.kill_signal)
	    goto failexit;
	
	
	if (sigs & notifysig) {
	    while ((nmsg = (struct notify_msg *)GetMsg(xsd->x11task_notify_port))) {
		/* Add the messages to an internal list */
//		kprintf("MEASSAGE RECEIVED\n");
		
		switch (nmsg->notify_type) {
		
		case NOTY_WINCREATE: {
		    struct xwinnode * node;
// kprintf("RECEIVED CREATENOTIFY\n");		    
		    /* Maintain a list of open windows for the X11 event handler in x11.c */
		    
		    node = AllocMem(sizeof (struct xwinnode), MEMF_CLEAR);

		    if (NULL != node) {
		    
		    	node->xwindow = nmsg->xwindow;
			node->bmobj   = nmsg->bmobj; 
		    	AddTail( (struct List *)&xwindowlist, (struct Node *)node );
			
		    } else {
		    	kprintf("!!!! CANNOT GET MEMORY FOR X11 WIN NODE\n");
		    	kill(getpid(), 19);
		    }
		    
		    ReplyMsg((struct Message *)nmsg);
		    break; }
		
		case NOTY_MAPWINDOW: {
		    /* BOOL found = FALSE; */
		    struct xwinnode *node;
		    
	            XMapRaised (nmsg->xdisplay, nmsg->xwindow);

		    #if 0 /* stegerg: this was, when XMapRaised was still called in onbitmap.c */
		    
		    /* If the window has allready been mapped, then just reply the message */
		    ForeachNode(&xwindowlist, node) {
		    	if (nmsg->xwindow == node->xwindow) {
			    if (node->window_mapped) {
			    	ReplyMsg((struct Message *)nmsg);
				found = TRUE;
			    }
			}
		    
		    }
		    
		    #endif
		    
		    /*if (!found) {*/
			AddTail((struct List *)&nmsg_list, (struct Node *)nmsg);			
		    /*}*/
		    
		    /* Do not reply message yet */
		    break; }
		
		case NOTY_WINDISPOSE: {
		    struct xwinnode *node, *safe;
		    
		    
		    ForeachNodeSafe(&xwindowlist, node, safe) {
		    	if (node->xwindow == nmsg->xwindow) {
			     Remove((struct Node *)node);
				
			     FreeMem(node, sizeof (struct xwinnode));
			     
			}
		    }
		    
		    ReplyMsg((struct Message *)nmsg);
		
		    break; }
		    
		 } /* switch() */
	    } /* while () */
	    
	    continue;
	    
	} /* if (message from notify port) */



 	for (;;)	    
	{
	    BOOL window_found = FALSE;
	    struct xwinnode *node;
	    int pending;



LX11	
D(bug("Calling XPending\n"));
	    pending = XPending (xsd->display);
UX11	    

// kprintf("pending: %d\n");
	    if (pending == 0)
	    	break;

	
LX11
	    XNextEvent (xsd->display, &event);
UX11

	    D(bug("Got Event for X=%d\n", event.xany.window));

// kprintf("GOT EVENT FROM X: %d\n", event.type);		    
	    if (event.type == MappingNotify) {
LX11
		    XRefreshKeyboardMapping ((XMappingEvent*)&event);
UX11
		    continue;
	    }
	    
	    
	    ForeachNode( &xwindowlist, node)
	    {
	        if (node->xwindow == event.xany.window)
		{
		    window_found = TRUE;
		    break;
		}
	    }
	    
	    
	    if (window_found)
	    {
	        D(bug("Got event for window %x\n", event.xany.window));
	    	switch (event.type)
	    	{
	    	case GraphicsExpose:
	    	case Expose:
		    break;

	        case ConfigureNotify:
		    break;

	    	case ButtonPress:
	        case ButtonRelease:
	    	case MotionNotify:
		    D(bug("Motionnotify event\n"));

		
	    	    ObtainSemaphoreShared( &xsd->sema );
		    if (xsd->mousehidd)
			Hidd_X11Mouse_HandleEvent(xsd->mousehidd, &event);
		    ReleaseSemaphore( &xsd->sema );
		    break;
		    
		case FocusOut:
LX11
		    XAutoRepeatOn(xsd->display);
UX11		    
ObtainSemaphoreShared(&xsd->sema);
		    /* Call the user supplied callback func, if supplied */
		    if (NULL != xsd->activecallback) {
		    	xsd->activecallback(xsd->callbackdata, node->bmobj, FALSE);
		    }
ReleaseSemaphore(&xsd->sema);
		    break;
		    
		case FocusIn:
ObtainSemaphoreShared(&xsd->sema);
		    /* Call the user supplied callback func, if supplied */
		    if (NULL != xsd->activecallback) {
		    	xsd->activecallback(xsd->callbackdata, node->bmobj, TRUE);
		    }
ReleaseSemaphore(&xsd->sema);
		    break;

	    	case KeyPress:
LX11
    		    XAutoRepeatOff(XSD(cl)->display);
UX11		    
	    	    ObtainSemaphoreShared( &xsd->sema );
		    if (xsd->kbdhidd)
		    {
			Hidd_X11Kbd_HandleEvent(xsd->kbdhidd, &event);
		    }
	    	
		    ReleaseSemaphore( &xsd->sema );
		    break;
		
		 
	    	case KeyRelease:
LX11
		    XAutoRepeatOn(XSD(cl)->display);
UX11		    
	    	    ObtainSemaphoreShared( &xsd->sema );
		    if (xsd->kbdhidd)
		    {
			Hidd_X11Kbd_HandleEvent(xsd->kbdhidd, &event);
		    }
	    	
		    ReleaseSemaphore( &xsd->sema );
		    break;


	   	case EnterNotify:
		    break;

	    	case LeaveNotify:
		    break;
		    
		case MapNotify: {
		
		    XMapEvent *me;
		    struct notify_msg *nmsg, *safe;
		    struct xwinnode *node;
		    
		    BOOL found = FALSE;
		    
		    me = (XMapEvent *)&event;
		    
		    ForeachNodeSafe(&nmsg_list, nmsg, safe) {
		    	if (me->window == nmsg->xwindow) {
			     /*  The window has now been mapped.
			         Send reply to app */
				 
		 	     found = TRUE;
			     Remove((struct Node *)nmsg);
			     ReplyMsg((struct Message *)nmsg);

			}
		    }
		    
		    /* Find it in thw window list and mark it as mapped */
		    
		    ForeachNode(&xwindowlist, node) {
		    	if (node->xwindow == me->window) {
			
			    node->window_mapped = TRUE;
			
			}
		    }
		    
		     
		    break; }

         	case ClientMessage:
            	    if (event.xclient.data.l[0] == xsd->delete_win_atom)
		    {
		        kill(getpid(), SIGINT);
		    }
		    break;

	        } /* switch (X11 event type) */
		
	    } /* if (is event for HIDD window) */


    	} /* while (events from X)  */
    	
    } /* Forever */
    
failexit:
#warning Also try to free window node list ?

    if (NULL != xsd->x11task_notify_port)
	DeleteMsgPort(xsd->x11task_notify_port);
		

#if (!NOUNIXIO)
    if (NULL != unixio_port)
    	DeleteMsgPort(unixio_port);
	
    if (NULL != unixio)
    	DisposeObject(unixio);
#endif
     Signal(xtp.parent, xtp.fail_signal);
    
}

struct Task *create_x11task( struct x11task_params *params, struct ExecBase *ExecBase)
{
    struct Task *task;
    APTR stack;
    
    task = AllocMem(sizeof (struct Task), MEMF_PUBLIC|MEMF_CLEAR);
    if (task)
    {
    	NEWLIST(&task->tc_MemEntry);
    	task->tc_Node.ln_Type=NT_TASK;
    	task->tc_Node.ln_Name= XTASK_NAME;
    	task->tc_Node.ln_Pri = XTASK_PRIORITY;

    	stack=AllocMem(XTASK_STACKSIZE, MEMF_PUBLIC);
    	if(stack != NULL)
    	{
	    task->tc_SPLower=stack;
	    task->tc_SPUpper=(BYTE *)stack + XTASK_STACKSIZE;

#if AROS_STACK_GROWS_DOWNWARDS
	    task->tc_SPReg = (BYTE *)task->tc_SPUpper-SP_OFFSET-sizeof(APTR);
	    ((APTR *)task->tc_SPUpper)[-1] = params;
#else
	    task->tc_SPReg=(BYTE *)task->tc_SPLower-SP_OFFSET + sizeof(APTR);
	    *(APTR *)task->tc_SPLower = params;
#endif
	    
	    /* You have to clear signals first. */
	    SetSignal(0, params->ok_signal | params->fail_signal);

	    if(AddTask(task, x11task_entry, NULL) != NULL)
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
