/*
    (C) 1997 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/
#include "locale_intern.h"

/*****************************************************************************

    NAME */
#include <clib/locale_protos.h>

	AROS_LH3(struct Catalog *, OpenCatalogA,

/*  SYNOPSIS */
	AROS_LHA(struct Locale  *, locale, A0),
	AROS_LHA(STRPTR          , name, A1),
	AROS_LHA(struct TagItem *, tags, A2),

/*  LOCATION */
	struct Library *, LocaleBase, 25, Locale)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	27-11-96    digulla automatically created from
			    locale_lib.fd and clib/locale_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct Library *,LocaleBase)
    extern void aros_print_not_implemented (char *);

    aros_print_not_implemented ("OpenCatalogA");

    AROS_LIBFUNC_EXIT
} /* OpenCatalogA */
