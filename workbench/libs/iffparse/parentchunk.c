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

	AROS_LH1(struct ContextNode *, ParentChunk,

/*  SYNOPSIS */
	AROS_LHA(struct ContextNode *, contextNode, A0),

/*  LOCATION */
	struct Library *, IFFParseBase, 30, IFFParse)

/*  FUNCTION
	Returns a pointer to the parent context node to the given
	one on the context node stack. The parent context node
	represents the chunk enclosing the chunk given.
	This can be use together with CurrentChunk() to iterate the
	context node stack top-down.

    INPUTS
	contextNode  - pointer to a context node.

    RESULT
	parent	    -  pointer to the parent context node or NULL if none.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	CurrentChunk()

    INTERNALS

    HISTORY
  27-11-96    digulla automatically created from
	  iffparse_lib.fd and clib/iffparse_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct Library *,IFFParseBase)

    struct ContextNode *parentcn;

    /* Get the parent of this contextnode. The contextstack
      is simulated via AddHead/RemHead so we should use
      .mln_Succ to get the parent
    */
    parentcn = (struct ContextNode*)contextNode->cn_Node.mln_Succ;

    /* If the parent of the found node is 0 (mlh_Tail field
       in struct MinList, then parentcn is the default contextnode,
       which the user not should have access to
    */

    if (!parentcn->cn_Node.mln_Succ)
	parentcn = NULL;

    return (parentcn);

    AROS_LIBFUNC_EXIT
} /* ParentChunk */
