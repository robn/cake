/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: Shortcut OpenLibrary call for system modules
    Lang: english
*/

#include <exec/types.h>
#include <aros/libcall.h>
#include <proto/exec.h>

char *libnames[] =
{
    "graphics.library",
    "layers.library",
    "intuition.library",
    "dos.library",
    "icon.library",
    "expansion.library",
    "utility.library",
    "keymap.library",
    "gadtools.library",
    "workbench.library"
};

char *copyrights[] =
{
    "AMIGA Replacement Operating System (AROS)",
    "Copyright � 1995-1997 ",
    "AROS - The Amiga Replacement OS  ",
    "Other parts � by respective owners.",
    "ALPHA ",
    "exec.library",
    "exec 40.101 (2.2.96)\013\010"
};

/*****i* exec.library/TaggedOpenLibrary **************************************

	TaggedOpenLibrary -- open a library by tag (V39)

    NAME */
	AROS_LH1(APTR, TaggedOpenLibrary,

/*  SYNOPSIS */
	AROS_LHA(LONG, tag, D0),

/*  LOCATION */
	struct ExecBase *, SysBase, 135, Exec)

/*  FUNCTION
	Opens a library given by tag. This is mainly meant as a shortcut so
	other system modules don't have to contain the complete library name
	string, to save ROM space. Additionaly, this call can be used to get a
	pointer to one of the system copyright notices and other strings.

	All libraries will be opened with version number 0.

	If the library cannot be opened the first try, this function calls
	FindResident and InitResident on the library, and tries again.

    INPUTS
	tag -  Which library or text string to return.

    RESULT
	Pointer to library or pointer to text string.

    NOTES
	This is an *INTERNAL* function, and is only meant to provide backwards
	compatibility until all original Amiga system ROM modules that use it
	have been implemented as part of AROS. This function *WILL BE REMOVED*
	in the future. *DO NOT USE!* This can not be emhasized enough. This
	also applies to AROS system programmers.

    EXAMPLE

    BUGS

    SEE ALSO
	OpenLibrary(), FindResident(), InitResident()

    INTERNALS
	No checks are made on the validity of the tag.

    HISTORY

******************************************************************************/
{
    struct Library *lib;
    struct Resident *res;

    if(tag > 0)
    {
	/*
	    Try to open the library. If it opened, return.
	*/
	if((lib = OpenLibrary(libnames[tag-1], 0))) return (APTR)lib;

	/*
	    If it didn't open, FindResident(), InitResident(), and then
	    try to open it again.
	*/
	if(!(res = FindResident(libnames[tag-1]))) return 0;
	InitResident(res, NULL);
	if((lib = OpenLibrary(libnames[tag-1], 0))) return (APTR)lib;
    }

    if(tag < 0) return( (APTR)copyrights[(-tag)-1] );

    /*
	If we get here, tag must be 0, or the lib didn't open.
    */
    return 0;

} /* TaggedOpenLibrary */
