/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: ANSI C function strchr()
    Lang: english
*/
#include <stdio.h>

/*****************************************************************************

    NAME */
#include <string.h>

	char * strchr (

/*  SYNOPSIS */
	const char * str,
	int	     c)

/*  FUNCTION
	Searches for a character in a string.

    INPUTS
	str - Search this string
	c - Look for this character

    RESULT
	A pointer to the first occurence of c in str or NULL if c is not
	found in str.

    NOTES

    EXAMPLE
	char buffer[64];

	strcpy (buffer, "Hello ");

	// This returns a pointer to the first l in buffer.
	strchr (buffer, 'l');

	// This returns NULL
	strchr (buffer, 'x');

    BUGS

    SEE ALSO
	strrchr()

    INTERNALS

    HISTORY

******************************************************************************/
{
    while (*str)
    {
	if (*str == c)
	    return ((char *)str);

	str ++;
    }

    return NULL;
} /* strchr */
