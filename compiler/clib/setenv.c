/*
    (C) 2001 AROS - The Amiga Research OS
    $Id$

    Desc: ANSI C function getenv()
    Lang: english
*/

#include <proto/dos.h>
#include <dos/var.h>

#include <errno.h>

/*****************************************************************************

    NAME */
#include <stdlib.h>

	char *setenv (

/*  SYNOPSIS */
	const char *name,
	const char *value,
	int         overwrite)
/*  FUNCTION
	 Change or add an environment variable.

    INPUTS
	name      - Name of the environment variable,
	value     - Value wich the variable must be set or changed to.
	overwrite - If non-zero then, if a variable with the name name already
	            exists, its value is changet to value, otherwise is not
		    changed
    RESULT
       Returns zero on success, or -1 if there was insufficient
       space in the environment.

    NOTES
        This function must not be used in a shared library.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	4.19.2001 falemagn created

******************************************************************************/
{
    if (!overwrite && FindVar(name, LV_VAR))
    	return 0;

    return -!SetVar(name, value, -1, LV_VAR );
} /* setenv */

