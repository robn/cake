/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Reply a message
    Lang: english
*/
#include "exec_intern.h"
#include <aros/libcall.h>
#include <exec/ports.h>
#include <proto/exec.h>

/*****************************************************************************

    NAME */

	AROS_LH1(void, ReplyMsg,

/*  SYNOPSIS */
	AROS_LHA(struct Message *, message, A1),

/*  LOCATION */
	struct ExecBase *, SysBase, 63, Exec)

/*  FUNCTION
	Send a message back to where it came from. It's generally not
	wise to access the fields of a message after it has been replied.

    INPUTS
	message - a message got with GetMsg().

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	WaitPort(), GetMsg(), PutMsg()

    INTERNALS

******************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct MsgPort *port;

    /* handle FASTCALL before doing locking or anything else. yes, there's a
     * potential race here if some task was to change mn_ReplyPort before/as
     * we read it. thats why we fetch it again further down, after Disable().
     * all bets are of when using FASTCALL */
    port = message->mn_ReplyPort;

    if (port != NULL && port->mp_Flags & PA_FASTCALL)
    {
        if (port->mp_SoftInt == NULL || ((struct Interrupt *) port->mp_SoftInt)->is_Code == NULL)
            return;

        ASSERT_VALID_PTR(port->mp_SoftInt);
        ASSERT_VALID_PTR(((struct Interrupt *) port->mp_SoftInt)->is_Code);

        message->mn_Node.ln_Type = NT_REPLYMSG;

        /* call the "interrupt" with the message as an argument */
        AROS_UFC4(void, ((struct Interrupt *) port->mp_SoftInt)->is_Code,
            AROS_UFCA(APTR,              ((struct Interrupt *) port->mp_SoftInt)->is_Data, A1),
            AROS_UFCA(ULONG_FUNC,        (ULONG_FUNC)((struct Interrupt *) port->mp_SoftInt)->is_Code, A5),
            AROS_UFCA(struct Message *,  message,                                          D0),
            AROS_UFCA(struct ExecBase *, SysBase,                                          A6));

        return;
    }

    /* Protect the message against access by other tasks. */
    Disable();

    /* Get replyport */
    port=message->mn_ReplyPort;

    /* Not set? Only mark the message as no longer sent. */
    if(port==NULL)
	message->mn_Node.ln_Type=NT_FREEMSG;
    else
    {
	/* Mark the message as replied */
	message->mn_Node.ln_Type=NT_REPLYMSG;

	/* Add it to the replyport's list */
	AddTail(&port->mp_MsgList,&message->mn_Node);

	if(port->mp_SigTask)
	{
	    /* And trigger the arrival action. */
	    switch(port->mp_Flags&PF_ACTION)
	    {
		case PA_SIGNAL:
		    /* Send a signal */
		    Signal((struct Task *)port->mp_SigTask,1<<port->mp_SigBit);
		    break;

		case PA_SOFTINT:
		    /* Raise a software interrupt */
		    Cause((struct Interrupt *)port->mp_SoftInt);
		    break;

		case PA_IGNORE:
		    /* Do nothing */
		    break;

                case PA_CALL:
                    /* Call the function in mp_SigTask. */
                    AROS_UFC2(void, port->mp_SigTask,
                        AROS_UFCA(struct MsgPort *,  port,    D0),
                        AROS_UFCA(struct ExecBase *, SysBase, A6));
                    break;
	    }
	}
    }

    /* All done */
    Enable();
    AROS_LIBFUNC_EXIT
} /* ReplyMsg() */

