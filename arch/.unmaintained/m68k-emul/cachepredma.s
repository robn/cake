/*
    (C) 1995-2000 AROS - The Amiga Research OS
    $Id$

    Desc: Exec function CachePreDMA
    Lang: english
*/

/******************************************************************************

    NAME
	AROS_LH0(void, CachePreDMA,

    LOCATION
	struct ExecBase *, SysBase, 127, Exec)

    FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

******************************************************************************/

	#include "machine.i"

	.text
	.balign 16
	.globl	AROS_SLIB_ENTRY(CachePreDMA,Exec)
	.type	AROS_SLIB_ENTRY(CachePreDMA,Exec),@function
AROS_SLIB_ENTRY(CachePreDMA,Exec):
	/* Dummy */
	rts

