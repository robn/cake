/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$
*/

/******************************************************************************
 
    NAME
 
 	AROS_LH3(void, CachePostDMA,
 
    SYNOPSIS
 	AROS_LHA(APTR,    address, A0),
 	AROS_LHA(ULONG *, length,  A1),
 	AROS_LHA(ULONG,   flags,  D0),
 
    LOCATION
 	struct ExecBase *, SysBase, 128, Exec)
 
    FUNCTION
 	Do everything necessary to make CPU caches aware that a DMA has
 	happened.
 
    INPUTS
 	address - Virtual address of memory affected by the DMA
 	*length - Number of bytes affected
 	flags	- DMA_NoModify	  - Indicate that the memory did not change.
 		  DMA_ReadFromRAM - Indicate that the DMA goes from RAM
 				    to the device. Set this bit in bot calls.
 
    RESULT
 
    NOTES
 	DMA must follow a call to CachePreDMA() and must be followed
 	by a call to CachePostDMA().
 
    EXAMPLE
 
    BUGS
 
    SEE ALSO
 	CachePreDMA()
 
    INTERNALS
 
    HISTORY
 
******************************************************************************/

/*
   XDEF AROS_SLIB_ENTRY(CachePostDMA,Exec)   	; for 68000/10/20
   XDEF AROS_SLIB_ENTRY(CachePostDMA_30,Exec)	; for 68030+
   XDEF AROS_SLIB_ENTRY(CachePostDMA_40,Exec)	; for 68040/68060
*/

	#include "machine.i"

	.text
	.balign 4
	.globl	AROS_SLIB_ENTRY(CachePostDMA,Exec)
	.type	AROS_SLIB_ENTRY(CachePostDMA,Exec),@function
AROS_SLIB_ENTRY(CachePostDMA,Exec):
	rts

	.text
	.balign 4
	.globl	AROS_SLIB_ENTRY(CachePostDMA_30,Exec)
	.type	AROS_SLIB_ENTRY(CachePostDMA_30,Exec),@function
AROS_SLIB_ENTRY(CachePostDMA_30,Exec):
	btst.l	#DMAB_NoModify,d0	/* Has DMA modified data in mem? */
	bne.s	cpd_30_end		/* nope, just exit */
	move.l	a5,a1			/* save a5 */
	lea.l	cachepostdmasup_30(pc),a5
	jmp	Supervisor(a6)

cpd_30_end:
	rts

cachepostdmasup_30:
	/* A DMA device has changed data in main memory. We have to clear
	   the data cache, so we get the chance to see this new data. */
	or.w	#0x0700,sr	/* Disable interrupts */
	movec	cacr,d0
	bset.l	#11,d0		/* Set CD Clear Data cache bit */
	movec	d0,cacr
	rte

	.text
	.balign 4
	.globl	AROS_SLIB_ENTRY(CachePostDMA_40,Exec)
	.type	AROS_SLIB_ENTRY(CachePostDMA_40,Exec),@function
AROS_SLIB_ENTRY(CachePostDMA_40,Exec):
	btst.l	#DMAB_NoModify,d0	/* Has DMA modified data in mem? */
	bne.s	cpd_40_end		/* nope, just exit */
	move.l	a5,a1			/* save a5 */
	lea.l	cachepostdmasup_40(pc),a5
	jmp	Supervisor(a6)

cpd_40_end:
	rts

cachepostdmasup_40:
	/* A DMA device has changed data in main memory. We have to invalidate
	   the data cache, so we get the chance to see this new data. */
	cinva	dc
	rte
