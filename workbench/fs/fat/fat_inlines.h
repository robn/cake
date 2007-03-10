/*
 * fat.handler - FAT12/16/32 filesystem handler
 *
 * Copyright � 2006 Marek Szyprowski
 * Copyright � 2007 The AROS Development Team
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 *
 * $Id$
 */

#ifndef __FAT_HANDLER_INLINES_H
#define __FAT_HANDLER_INLINES_H

static inline LONG TestLock(struct ExtFileLock *fl)
{
	if (fl == 0 && glob->sb == NULL)
	{
		if (glob->disk_inserted == FALSE)
			return ERROR_NO_DISK;
		else
			return ERROR_NOT_A_DOS_DISK;
	}
 
	if (glob->sb == NULL || glob->disk_inhibited || (fl && fl->fl_Volume != MKBADDR(glob->sb->doslist)))
		return ERROR_DEVICE_NOT_MOUNTED;

	if (fl && fl->magic != ID_FAT_DISK)
		return ERROR_OBJECT_WRONG_TYPE;

	return 0;
}

#define SkipColon(name, namelen) {		 \
	int i;                      		 \
	for (i=0; i<namelen; i++)   		 \
		if (name[i] == ':')     		 \
		{                       		 \
			namelen = namelen - (i+1);   \
			name = &name[i+1];  		 \
			break;              		 \
		}                       		 \
}

/* Mem... */

static inline void *FS_AllocMem(ULONG bytes)
{
	return AllocVecPooled(glob->mempool, bytes);
}

static inline void FS_FreeMem(void *mem)
{
	FreeVecPooled(glob->mempool, mem);
}


static inline UBYTE *FS_AllocBlock()
{
#ifdef __DEBUG_IO__
	kprintf("\tAllocating block buffer\n");
#endif
	 return FS_AllocMem(glob->sb->sectorsize);
}

static inline void FS_FreeBlock(UBYTE *block)
{
#ifdef __DEBUG_IO__
	kprintf("\tFreeing block buffer\n");
#endif
	FS_FreeMem(block);
}
 

static inline ULONG Cluster2Sector(struct FSSuper *sb, ULONG n)
{
	return ((n-2) << sb->cluster_sectors_bits) + sb->first_data_sector;
}

static inline ULONG GetFatEntry(ULONG n)
{
	return glob->sb->func_get_fat_entry(glob->sb, n);
}

static inline ULONG GetFirstCluster(struct DirEntry *de)
{
	return LE16(de->first_cluster_lo) | (((ULONG)LE16(de->first_cluster_hi)) << 16);
}

/* IO layer */

static inline LONG FS_GetBlock (ULONG n, UBYTE *dst)
{
#ifndef API_LIBDEVIO
    glob->diskioreq->iotd_Req.io_Command = CMD_READ;
    glob->diskioreq->iotd_Req.io_Data = dst;
    glob->diskioreq->iotd_Req.io_Offset = n * glob->blocksize;
    glob->diskioreq->iotd_Req.io_Length = glob->blocksize;
    glob->diskioreq->iotd_Req.io_Flags = IOF_QUICK;
    
    DoIO((struct IORequest *) glob->diskioreq);

    return glob->diskioreq->iotd_Req.io_Error;
#else
	LONG err = ReadBlocks(glob->dio, n, dst, 1);
#ifdef __DEBUG_IO__
	kprintf("\t\tReadBlock: %ld\n", n);
	if (err)
		kprintf("\tReadBlock failed: %ld err %ld\n", err, IoErr());
#endif
	return err;
#endif
}

static inline LONG FS_GetBlocks (ULONG n, UBYTE *dst, ULONG count)
{
#ifndef API_LIBDEVIO
    glob->diskioreq->iotd_Req.io_Command = CMD_READ;
    glob->diskioreq->iotd_Req.io_Data = dst;
    glob->diskioreq->iotd_Req.io_Offset = n * glob->blocksize;
    glob->diskioreq->iotd_Req.io_Length = count * glob->blocksize;
    glob->diskioreq->iotd_Req.io_Flags = IOF_QUICK;
    
    DoIO((struct IORequest *) glob->diskioreq);

    return glob->diskioreq->iotd_Req.io_Error;
#else
	LONG err = ReadBlocks(glob->dio, n, dst, count);
#ifdef __DEBUG_IO__
	kprintf("\t\tReadBlocks: %ld count %ld\n", n, count);
	if (err)
		kprintf("\tReadBlock failed: %ld err %ld\n", err, IoErr());
#endif
	return err;
#endif
}

static inline LONG FS_PutBlock (ULONG n, UBYTE *dst)
{
#ifndef API_LIBDEVIO
    glob->diskioreq->iotd_Req.io_Command = CMD_WRITE;
    glob->diskioreq->iotd_Req.io_Data = dst;
    glob->diskioreq->iotd_Req.io_Offset = n * glob->blocksize;
    glob->diskioreq->iotd_Req.io_Length = glob->blocksize;
    glob->diskioreq->iotd_Req.io_Flags = IOF_QUICK;
    
    DoIO((struct IORequest *) glob->diskioreq);

    return glob->diskioreq->iotd_Req.io_Error;
#else
	LONG err = WriteBlocks(glob->dio, n, dst, 1);
#ifdef __DEBUG_IO__
	kprintf("\tWriteBlock: %ld\n", n);
	if (err)
		kprintf("\tWriteBlock failed: %ld err %ld\n", err, IoErr());
#endif
	return err;
#endif
}


#endif
