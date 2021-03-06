/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$
*/

/*****************************************************************************
 
    NAME
 
 	AROS_LH3(APTR, CachePreDMA,
 
    SYNOPSIS
 	AROS_LHA(APTR,    address, A0),
 	AROS_LHA(ULONG *, length,  A1),
 	AROS_LHA(ULONG,   flags,  D0),
 
    LOCATION
 	struct ExecBase *, SysBase, 127, Exec)
 
    FUNCTION
 	Do everything necessary to make CPU caches aware that a DMA will happen.
 	Virtual memory systems will make it possible that your memory is not at
 	one block and not at the address you thought. This function gives you
 	all the information you need to split the DMA request up and to convert
 	virtual to physical addresses.
 
    INPUTS
 	address - Virtual address of memory affected by the DMA
 	*length - Number of bytes affected
 	flags	- DMA_Continue	  - This is a call to continue a request that
 				    was broken up.
 		  DMA_ReadFromRAM - Indicate that the DMA goes from RAM
 				    to the device. Set this bit in bot calls.
 
    RESULT
 	The physical address in memory.
 	*length contains the number of contiguous bytes in physical memory.
 
    NOTES
 	DMA must follow a call to CachePreDMA() and must be followed
 	by a call to CachePostDMA().
 
    EXAMPLE
 
    BUGS
 
    SEE ALSO
 	CachePostDMA()
 
    INTERNALS
 
    HISTORY
 
******************************************************************************/

/*
   XDEF AROS_SLIB_ENTRY(CachePreDMA,Exec)   	; for 68000/10/20/30
   XDEF AROS_SLIB_ENTRY(CachePreDMA_40,Exec)	; for 68040+
*/

	#include "machine.i"

	.text
	.balign 4
	.globl	AROS_SLIB_ENTRY(CachePreDMA,Exec)
	.type	AROS_SLIB_ENTRY(CachePreDMA,Exec),@function
AROS_SLIB_ENTRY(CachePreDMA,Exec):
	move.l	a0,d0	/* return input address */
	rts

	.text
	.balign 4
	.globl	AROS_SLIB_ENTRY(CachePreDMA_40,Exec)
	.type	AROS_SLIB_ENTRY(CachePreDMA_40,Exec),@function
AROS_SLIB_ENTRY(CachePreDMA_40,Exec):
	move.l	a5,a1		/* save a5 */
	lea.l	cachepredmasup_40(pc),a5
	jmp	Supervisor(a6)

cachepredmasup_40:
	cpusha	dc	/* Push dirty data cache lines to memory and invalidate cache */
	cinva	dc	/* 68060 invalidates depending on DPI (Disable CPUSH invalidation)
			   bit of CACR. Force an invalidation with CINV. */
	move.l	a0,d0	/* return input address */
	rte
