/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/
#include <exec/memory.h>
#include <clib/exec_protos.h>

/*****************************************************************************

    NAME */
#include <exec/io.h>
#include <clib/alib_protos.h>

	struct IORequest * CreateExtIO (

/*  SYNOPSIS */
	struct MsgPort * port,
	ULONG		 iosize)

/*  FUNCTION
	Create an extended IORequest structure. This structure can
	be freed with DeleteExtIO().

    INPUTS
	port - MsgPort to be signaled on events
	iosize - Size of the structure

    RESULT
	A pointer to the new IORequest structure.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	CreateStdIO(), DeleteExtIO()

    INTERNALS

    HISTORY

******************************************************************************/
{
    struct IORequest *ioreq=NULL;

    if (port && (ioreq = AllocMem (iosize, MEMF_CLEAR|MEMF_PUBLIC)))
    {
	/* Initialize the structure */
	ioreq->io_Message.mn_Node.ln_Type = NT_MESSAGE;
	ioreq->io_Message.mn_ReplyPort	  = port;
	ioreq->io_Message.mn_Length	  = iosize;
    }

    return ioreq;
} /* CreateExtIO */

