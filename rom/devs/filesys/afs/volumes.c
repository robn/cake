/*
    Copyright � 1995-2008, The AROS Development Team. All rights reserved.
    $Id$
*/

/*
 * -date------ -name------------------- -description-----------------------------
 * 26-dec-2007 [Tomasz Wiszkowski]      added disk validation
 * 04-jan-2008 [Tomasz Wiszkowski]      corrected tabulation
 * 05-jan-2008 [Tomasz Wiszkowski]		 removed requester prompting for validation
 * 												 to allow volume validation during boot-up
 */

#ifndef DEBUG
#define DEBUG 0
#endif

#include "os.h"
#include "bitmap.h"
#include "checksums.h"
#include "error.h"
#include "afsblocks.h"
#include "baseredef.h"
#include "validator.h"


BOOL mediumPresent(struct IOHandle *ioh) {
	return (ioh->ioflags & IOHF_DISK_IN)==IOHF_DISK_IN;
}

/*******************************************
 Name  : newMedium
 Descr.: build infos for a new medium
 Input : volume  -
 Output: 0 for success; error code otherwise
********************************************/
LONG newMedium(struct AFSBase *afsbase, struct Volume *volume) {
struct BlockCache *blockbuffer;
UWORD i;
LONG error;

	/* Check validity of root block first, since boot block may be left over
	   from an overwritten partition of a different size */
	blockbuffer=getBlock(afsbase, volume,volume->rootblock);
	if (blockbuffer == NULL)
		return ERROR_UNKNOWN;
	if (calcChkSum(volume->SizeBlock, blockbuffer->buffer) != 0 ||
		OS_BE2LONG(blockbuffer->buffer[BLK_SECONDARY_TYPE(volume)]) != ST_ROOT)
		return ERROR_NOT_A_DOS_DISK;

	blockbuffer=getBlock(afsbase, volume,0);
	if (blockbuffer == NULL)
		return ERROR_UNKNOWN;
	volume->dostype=OS_BE2LONG(blockbuffer->buffer[0]) & 0xFFFFFF00;
	if (volume->dostype != 0x444F5300)
	{
		blockbuffer=getBlock(afsbase, volume, 1);
		volume->dostype=OS_BE2LONG(blockbuffer->buffer[0]) & 0xFFFFFF00;
	}
	volume->dosflags = OS_BE2LONG(blockbuffer->buffer[0]) & 0xFF;
	if (volume->dostype != 0x444F5300)
		return ERROR_NOT_A_DOS_DISK;
	blockbuffer=getBlock(afsbase, volume,volume->rootblock);
	if (blockbuffer == NULL)
		return ERROR_UNKNOWN;
	for (i=0;i<=24;i++)
	{
		volume->bitmapblockpointers[i]=OS_BE2LONG
			(
				blockbuffer->buffer[BLK_BITMAP_POINTERS_START(volume)+i]
			);
	}
	volume->bitmapextensionblock=OS_BE2LONG
		(
			blockbuffer->buffer[BLK_BITMAP_EXTENSION(volume)]
		);
	if (!blockbuffer->buffer[BLK_BITMAP_VALID_FLAG(volume)])
	{
		/* since our disk is invalid at this point, *
		 * we want to spawn a new process that will *
		 * perform disk validation while we move on */

		volume->usedblockscount=0;
		volume->state = ID_VALIDATING;

		launchValidator(afsbase, volume);
	}

	/*
	 * it's safe to assume that the block is still there
	 */
	blockbuffer=getBlock(afsbase, volume,volume->rootblock);
	
	if (blockbuffer->buffer[BLK_BITMAP_VALID_FLAG(volume)])
	{
		blockbuffer->flags |= BCF_USED;	// won't be cleared until volume is ejected
		volume->usedblockscount=countUsedBlocks(afsbase, volume);
		volume->state = (volume->dosflags & 0xFC) ? ID_WRITE_PROTECTED : ID_VALIDATED;
	}
	error = osMediumInit(afsbase, volume, blockbuffer);
	if (error != 0)
		return error;
	/* for free block searching */
	volume->lastaccess=volume->rootblock;
	return 0;
}

/*******************************************
 Name  : initVolume
 Descr.: maybe a better name would be mountVolume
         allocate resources for a new mounted device
 Input : device      - device pointer
         blockdevice - name of blockdevice
         unit        - unit number of blockdevice
         devicedef   - medium geometry data
         error       - return error code
 Output: 0 on error (error set dos dos error);
         pointer to struct Volume on success
********************************************/
struct Volume *initVolume
	(
		struct AFSBase *afsbase,
		struct Device *device,
		CONST_STRPTR blockdevice,
		ULONG unit,
		struct DosEnvec *devicedef,
		ULONG *error
	)
{
struct Volume *volume;

	volume = AllocMem(sizeof(struct Volume),MEMF_PUBLIC | MEMF_CLEAR);
	if (volume != NULL)
	{
		volume->device = device;
		volume->ioh.blockdevice = blockdevice;
		volume->ioh.unit = unit;
		volume->ioh.flags = 0;
		volume->SizeBlock=devicedef->de_SizeBlock;
		if (devicedef->de_TableSize>=20)
			volume->bootblocks=devicedef->de_BootBlocks;
		else
			volume->bootblocks=devicedef->de_Reserved;
		volume->blockcache=initCache(afsbase, volume, devicedef->de_NumBuffers);
		if (volume->blockcache != NULL)
		{
			if (openBlockDevice(afsbase, &volume->ioh)!= NULL)
			{
				volume->countblocks =
					(
						(
							devicedef->de_HighCyl-devicedef->de_LowCyl+1
						)*devicedef->de_Surfaces*devicedef->de_BlocksPerTrack
					);
				volume->rootblock =(volume->countblocks-1+devicedef->de_Reserved)/2;
				volume->startblock=
						devicedef->de_LowCyl*
						devicedef->de_Surfaces*
						devicedef->de_BlocksPerTrack;
				volume->lastblock=
						(
							(devicedef->de_HighCyl+1)
							*devicedef->de_Surfaces
							*devicedef->de_BlocksPerTrack
						)-1;
				check64BitSupport(afsbase, volume);
				volume->ah.volume=volume;
				if (mediumPresent(&volume->ioh))
				{
					*error = newMedium(afsbase, volume);
				}
				else
					*error = 0;
				if ((*error == 0) || (*error == ERROR_NOT_A_DOS_DISK))
				{
					D(bug("[afs] initVolume: BootBlocks=%d\n",volume->bootblocks));
					D(bug("[afs] initVolume: RootBlock=%ld\n",volume->rootblock));
					volume->ah.header_block = volume->rootblock;
					return volume;
				}
			}
			else
			{
				*error=ERROR_NO_FREE_STORE;
			}
			freeCache(afsbase, volume->blockcache);
		}
		else
		{
			*error=ERROR_NO_FREE_STORE;
		}
		FreeMem(volume,sizeof(struct Volume));
	}
	else
		*error=ERROR_NO_FREE_STORE;
	return NULL;
}

/*******************************************
 Name  : uninitVolume
 Descr.: maybe a better name would be unmountVolume
         free resources allocated by initVolume
 Input : volume  - volume to unmount
 Output: -
********************************************/
void uninitVolume(struct AFSBase *afsbase, struct Volume *volume) {

	osMediumFree(afsbase, volume, TRUE);
	if (volume->blockcache != NULL)
		freeCache(afsbase, volume->blockcache);
	closeBlockDevice(afsbase, &volume->ioh);
	FreeMem(volume,sizeof(struct Volume));
}

/* vim: set noet ts=3 ai fdm=marker fmr={,} :*/
