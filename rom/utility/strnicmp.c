/*
    (C) 1995-97 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include <exec/types.h>
#include <aros/libcall.h>
#include "intern.h"

/*****************************************************************************

    NAME */
#include <proto/utility.h>

	AROS_LH3(LONG, Strnicmp,

/*  SYNOPSIS */
	AROS_LHA(STRPTR, string1, A0),
	AROS_LHA(STRPTR, string2, A1),
	AROS_LHA(LONG,   length,  D0),

/*  LOCATION */
	struct UtilityBase *, UtilityBase, 28, Utility)

/*  FUNCTION
	Compares two strings treating lower and upper case characters
	as identical up to a given maximum number of characters.

    INPUTS
	string1, string2 - The strings to compare.
	length		 - maximum number of characters to compare.

    RESULT
	<0  if string1 <  string2
	==0 if string1 == string2
	>0  if string1 >  string2

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    UBYTE c1, c2;

    /* 0 characters are always identical */
    if(!length)
	return 0;

    /* Loop as long as the strings are identical and valid. */
    do
    {
	/* Get characters, convert them to lower case. */
	c1=ToLower (*string1++);
	c2=ToLower (*string2++);
    }while(c1==c2&&c1&&--length);

    /* Get result. */
    return (LONG)c1-(LONG)c2;
    AROS_LIBFUNC_EXIT
} /* Strnicmp */
