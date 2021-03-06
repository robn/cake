/*
        Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/
#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/rexxsyslib.h>
#include <rexx/storage.h>
#include <rexx/errors.h>

/*****************************************************************************

    NAME */

        BOOL CheckRexxMsg(

/*  SYNOPSIS */
	struct RexxMsg * msg)

/*  FUNCTION
        Check to see if provided message was generated by the rexx interpreter

    INPUTS
        msg - The message to check

    RESULT
        Wether this message is OK.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        SetRexxVar(), GetRexxVar()

    INTERNALS
        This function creates a rexx message that is sent to the AREXX
        port with a RXCHECKMSG command.


*****************************************************************************/
{
    struct Library *RexxSysBase = NULL;
    struct RexxMsg *msg2 = NULL, *msg3;
    struct MsgPort *port = NULL, *rexxport;
    BOOL retval = FALSE;
    
    RexxSysBase = OpenLibrary("rexxsyslib.library", 0);
    if (RexxSysBase == NULL) goto cleanup;
    
    if (!IsRexxMsg(msg)) goto cleanup;
    rexxport = FindPort("REXX");
    if (rexxport==NULL) goto cleanup;
  
    port = CreateMsgPort();
    if (port == NULL) goto cleanup;
    msg2 = CreateRexxMsg(port, NULL, NULL);
    if (msg2==NULL) goto cleanup;
    msg2->rm_Private1 = msg->rm_Private1;
    msg2->rm_Private2 = msg->rm_Private2;
    msg2->rm_Action = RXCHECKMSG;
    
    PutMsg(rexxport, (struct Message *)msg2);
    msg3 = NULL;
    while (msg3!=msg2)
    {
	WaitPort(port);
	msg3 = (struct RexxMsg *)GetMsg(port);
	if (msg3!=msg2) ReplyMsg((struct Message *)msg3);
    }

    retval = msg3->rm_Result1==RC_OK;
    
cleanup:
    if (msg2!=NULL) DeleteRexxMsg(msg2);
    if (port!=NULL) DeletePort(port);
    if (RexxSysBase!=NULL) CloseLibrary(RexxSysBase);
    return retval;
} /* CheckRexxMsg */
