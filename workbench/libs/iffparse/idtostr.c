/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include "iffparse_intern.h"

/*****************************************************************************

    NAME */
#include <proto/iffparse.h>

	AROS_LH2(STRPTR, IDtoStr,

/*  SYNOPSIS */
	AROS_LHA(LONG  , id, D0),
	AROS_LHA(STRPTR, buf, A0),

/*  LOCATION */
	struct Library *, IFFParseBase, 45, IFFParse)

/*  FUNCTION

    INPUTS
	id  - pointer to an IFF chunk identfication code.
	buf  -	buffer into which the id will be stored. Should at least be 5 bytes.

    RESULT
	buf  -	pointer to the supplied buffer.

    NOTES
	Assumes that the supplied ID is stored in local byte order.

    EXAMPLE
	// Print the ID of the current contextnode

	UBYTE buf[5];
	struct ContextNode *cn;

	if (cn = CurrentChunk(iff)
	    printf
	    (
		"ID of current chunk: %s\n",
		IDtoStr(cn->cn_ID)
	    );


    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
  27-11-96    digulla automatically created from
	  iffparse_lib.fd and clib/iffparse_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct Library *,IFFParseBase)

    UBYTE *idbuf = (UBYTE*)&id;

/* If CPU is little endian, then we must "rotate" when writing to the string */

#if (AROS_BIG_ENDIAN == 0)
    buf[0] = idbuf[3];
    buf[1] = idbuf[2];
    buf[2] = idbuf[1];
    buf[3] = idbuf[0];
#else
/* Big endian CPU: no problems */

    buf[0] = idbuf[0];
    buf[1] = idbuf[1];
    buf[2] = idbuf[2];
    buf[3] = idbuf[3];
#endif

    buf[4] = 0;

    return buf;

    AROS_LIBFUNC_EXIT
} /* IDtoStr */
