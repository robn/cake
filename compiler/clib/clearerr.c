/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: ANSI C function clearerr()
    Lang: english
*/

/*****************************************************************************

    NAME */
#include <stdio.h>

	void clearerr (

/*  SYNOPSIS */
	FILE * stream)

/*  FUNCTION
	Clear EOF and error flag in a stream. You must call this for
	example after you have read the file until EOF, then appended
	something to it and want to continue reading.

    INPUTS
	stream - The stream to be reset.

    RESULT
	None.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	ferror(), clearerr()

    INTERNALS

    HISTORY

******************************************************************************/
{
    stream->flags &= ~(_STDIO_FILEFLAG_EOF | _STDIO_FILEFLAG_ERROR);
} /* clearerr */

