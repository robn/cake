/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/
#include "iffparse_intern.h"

/*****************************************************************************

    NAME */
#include <proto/iffparse.h>

	AROS_LH1(void, FreeIFF,

/*  SYNOPSIS */
	AROS_LHA(struct IFFHandle *, iff, A0),

/*  LOCATION */
	struct Library *, IFFParseBase, 9, IFFParse)

/*  FUNCTION
	Frees an IFFHandle struct previously allocated by AllocIFF.

    INPUTS
	iff - pointer to an IFFHandle struct.
    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	AllocIFF(), CloseIFF()

    INTERNALS

    HISTORY
  27-11-96    digulla automatically created from
	  iffparse_lib.fd and clib/iffparse_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct Library *,IFFParseBase)

    struct IntContextNode *cn;

    struct LocalContextItem  *node,
			    *nextnode;

  /* We should free the LCIs of the default context-node
      ( CollectionItems and such )
    */
    cn = (struct IntContextNode*)RootChunk(iff);

    node = (struct LocalContextItem*)cn->cn_LCIList.mlh_Head;

    while ((nextnode = (struct LocalContextItem*)node->lci_Node.mln_Succ))
    {
	PurgeLCI(node, IPB(IFFParseBase));

	node = nextnode;
    }


    FreeMem(iff,sizeof (struct IntIFFHandle));

    return;

    AROS_LIBFUNC_EXIT
} /* FreeIFF */
