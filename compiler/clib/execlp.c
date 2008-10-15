/*
    Copyright � 2008, The AROS Development Team. All rights reserved.
    $Id$

    POSIX function execlp().
*/

#include <aros/debug.h>
#include <errno.h>
#include <stdlib.h>

/*****************************************************************************

    NAME */
#include <unistd.h>

	int execlp(

/*  SYNOPSIS */
	const char *file, 
        const char *arg, ...)
        
/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	
    INTERNALS

******************************************************************************/
{
    va_list args;
    char *default_argv[] = { NULL };
    char **argv;

    if(arg != NULL)
    {
        int argc = 1;
        va_start(args,arg);
        while(va_arg(args,const char *) != NULL)
            argc++;
        va_end(args);
        
        argv = (char**) malloc(sizeof(char*) * (argc + 1));
        if(!argv)
        {
            errno = ENOMEM;
            return -1;
        }
        
        argv[0] = (char*) arg;
        int argi;
        va_start(args,arg);
        for(argi = 1; argi < argc; argi++)
            argv[argi] = va_arg(args,char *);
        va_end(args);
        argv[argc] = NULL;
    }
    else
	argv = default_argv;
    
    return execvp(file, argv);
} /* execlp() */
