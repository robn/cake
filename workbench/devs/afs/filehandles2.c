/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$
*/

#ifndef DEBUG
#define DEBUG 1
#endif

#include <proto/exec.h>
#include <proto/dos.h>

#include <aros/debug.h>
#include <aros/macros.h>

#include "filehandles2.h"
#include "afsblocks.h"
#include "bitmap.h"
#include "checksums.h"
#include "error.h"
#include "extstrings.h"
#include "filehandles1.h"
#include "hashing.h"
#include "misc.h"
#include "baseredef.h"

extern ULONG error;


/********************************************
 Name  : setHeaderData
 Descr.: set actual date for an object
 Input : afsbase     -
         volume      - 
         blockbuffer - header block of object
         ds          - datestamp to set
 Output: 0 for success; error code otherwise
*********************************************/
ULONG setHeaderDate
	(
		struct afsbase *afsbase,
		struct Volume *volume,
		struct BlockCache *blockbuffer,
		struct DateStamp *ds
	)
{

	D(bug
		(
			"afs.handler: setHeaderDate: for headerblock %ld\n",
			blockbuffer->blocknum)
		);
	blockbuffer->buffer[BLK_DAYS(volume)]=AROS_LONG2BE(ds->ds_Days);
	blockbuffer->buffer[BLK_MINS(volume)]=AROS_LONG2BE(ds->ds_Minute);
	blockbuffer->buffer[BLK_TICKS(volume)]=AROS_LONG2BE(ds->ds_Tick);
	blockbuffer->buffer[BLK_CHECKSUM]=0;
	blockbuffer->buffer[BLK_CHECKSUM]=AROS_LONG2BE
		(
			0-calcChkSum(volume->SizeBlock,blockbuffer->buffer)
		);
	writeBlock(afsbase, volume,blockbuffer);
	blockbuffer=getBlock
		(
			afsbase,
			volume,
			AROS_LONG2BE(blockbuffer->buffer[BLK_PARENT(volume)])
		);
	if (!blockbuffer)
		return ERROR_UNKNOWN;
	return writeHeader(afsbase, volume, blockbuffer);
}


/********************************************
 Name  : setDate
 Descr.: set actual date for an object
 Input : afsbase -
         ah      - filehandle name is relative to
         name    - name of object 
         ds      - datestamp to set
 Output: 0 for success; error code otherwise
*********************************************/
ULONG setDate
	(
		struct afsbase *afsbase,
		struct AfsHandle *ah,
		STRPTR name,
		struct DateStamp *ds
	)
{
ULONG block;
struct BlockCache *blockbuffer;

	D(bug("afs.handler: setData()\n"));
	blockbuffer=findBlock(afsbase, ah, name, &block);
	if (!blockbuffer)
		return error;
	return setHeaderDate(afsbase, ah->volume,blockbuffer,ds);
}

/********************************************
 Name  : setProtect
 Descr.: set protection bits for an object
 Input : afsbase -
         ah      - filehandle name is relative to
         name    - name of object
         mask    - protection bit mask
 Output: 0 for success; error code otherwise
*********************************************/
ULONG setProtect
	(
		struct afsbase *afsbase,
		struct AfsHandle *ah,
		STRPTR name,
		ULONG mask
	)
{
ULONG block;
struct BlockCache *blockbuffer;

	D(bug("afs.handler: setProtect(ah,%s,%ld)\n",name,mask));
	blockbuffer=findBlock(afsbase, ah, name, &block);
	if (!blockbuffer)
		return error;
	blockbuffer->buffer[BLK_PROTECT(ah->volume)]=AROS_LONG2BE(mask);
	return writeHeader(afsbase, ah->volume, blockbuffer);
}

/********************************************
 Name  : setComment
 Descr.: set comment for an object
 Input : afsbase -
         ah      - filehandle name is relative to
         name    - name of object
         comment - comment to set
 Output: 0 for success; error code otherwise
*********************************************/
ULONG setComment
	(
		struct afsbase *afsbase,
		struct AfsHandle *ah,
		STRPTR name,
		STRPTR comment
	)
{
ULONG block;
struct BlockCache *blockbuffer;

	D(bug("afs.handler: setComment(ah,%s,%s)\n",name,comment));
	blockbuffer=findBlock(afsbase, ah, name, &block);
	if (!blockbuffer)
		return error;
	StrCpyToBstr
		(
			comment,
			(APTR)((ULONG)blockbuffer->buffer+(BLK_COMMENT_START(ah->volume)*4))
		);
	return writeHeader(afsbase, ah->volume, blockbuffer);
}

/************************************************
 Name  : unLinkBlock
 Descr.: unlinks an entry from the directorylist
 Input : lastentry - node before entry to unlink
                     (directory itself (head) or last block
                     which HASHCHAIN points to the entry to unlink
         entry     - entry to unlink
 Output: - 
 Note  : unlink is only done in buffers
         nothing is written to disk!
************************************************/
void unLinkBlock
	(
		struct afsbase *afsbase,
		struct Volume *volume,
		struct BlockCache *lastentry,
		struct BlockCache *entry
	)
{
ULONG key;

	D(bug("afs.handler: unlinkBlock: unlinking %ld\n",entry->blocknum));
	// find the "member" where entry is linked
	// ->linked into hashchain or hashtable
	key=BLK_HASHCHAIN(volume);
	if (AROS_BE2LONG(lastentry->buffer[key])!=entry->blocknum)
		for (key=BLK_TABLE_START;key<=BLK_TABLE_END(volume);key++)
			if (AROS_BE2LONG(lastentry->buffer[key])==entry->blocknum)
				break;
	lastentry->buffer[key]=entry->buffer[BLK_HASHCHAIN(volume)];	//unlink block
	lastentry->buffer[BLK_CHECKSUM]=0;
	lastentry->buffer[BLK_CHECKSUM]=AROS_LONG2BE
		(
			0-calcChkSum(volume->SizeBlock, lastentry->buffer)
		);
}

/********************************************
 Name  : deleteObject
 Descr.: delete an object
 Input : afsbase -
         ah      - filehandle name is relative to
         name    - name of object to delete
 Output: 0 for success; error code otherwise
*********************************************/
ULONG deleteObject(struct afsbase *afsbase, struct AfsHandle *ah, STRPTR name) {
ULONG lastblock,key;
struct BlockCache *blockbuffer, *priorbuffer;

	D(bug("afs.handler: delete(ah,%s)\n",name));
	blockbuffer=findBlock(afsbase, ah,name,&lastblock);
	if (!blockbuffer)
		return error;
	if (findHandle(ah->volume, blockbuffer->blocknum))
		return ERROR_OBJECT_IN_USE;
	if (AROS_BE2LONG(blockbuffer->buffer[BLK_PROTECT(ah->volume)]) & FIBF_DELETE)
		return ERROR_DELETE_PROTECTED;
	/* if we try to delete a directory
      check if it is empty
	*/
	if (
			AROS_BE2LONG
				(blockbuffer->buffer[BLK_SECONDARY_TYPE(ah->volume)])==ST_USERDIR
		)
	{
		for (key=BLK_TABLE_START;key<=BLK_TABLE_END(ah->volume);key++)
		{
			if (blockbuffer->buffer[key])
				return ERROR_DIRECTORY_NOT_EMPTY;
		}
	}
	blockbuffer->flags |= BCF_USED;
	priorbuffer=getBlock(afsbase, ah->volume, lastblock);
	if (!priorbuffer)
	{
		blockbuffer->flags &= ~BCF_USED;
		return ERROR_UNKNOWN;
	}
	if (calcChkSum(ah->volume->SizeBlock, priorbuffer->buffer))
	{
		blockbuffer->flags &= ~BCF_USED;
		showError(afsbase, ERR_CHECKSUM,priorbuffer->blocknum);
		return ERROR_UNKNOWN;
	}
	priorbuffer->flags |= BCF_USED;
	unLinkBlock(afsbase, ah->volume,priorbuffer, blockbuffer);
	invalidBitmap(afsbase, ah->volume);
	writeBlock(afsbase, ah->volume,priorbuffer);
	markBlock(afsbase, ah->volume, blockbuffer->blocknum, -1);
	if (
			AROS_BE2LONG
				(blockbuffer->buffer[BLK_SECONDARY_TYPE(ah->volume)])==ST_FILE
		)
	{
		for (;;)
		{
			D(bug("afs.handler:   extensionblock=%ld\n",blockbuffer->blocknum));
			for
				(
					key=BLK_TABLE_END(ah->volume);
					(key>=BLK_TABLE_START) && (blockbuffer->buffer[key]);
					key--
				)
			{
				markBlock
					(
						afsbase,
						ah->volume,
						AROS_BE2LONG(blockbuffer->buffer[key]),
						-1
					);
			}
			if (!blockbuffer->buffer[BLK_EXTENSION(ah->volume)])
				break;
			// get next extensionblock
			blockbuffer->flags &= ~BCF_USED;
			blockbuffer=getBlock
				(
					afsbase,
					ah->volume,
					AROS_BE2LONG(blockbuffer->buffer[BLK_EXTENSION(ah->volume)])
				);
			if (!blockbuffer)
			{
				priorbuffer->flags &= ~BCF_USED;
				return ERROR_UNKNOWN;
			}
			if (calcChkSum(ah->volume->SizeBlock, blockbuffer->buffer))
			{
				priorbuffer->flags &= ~BCF_USED;
				showError(afsbase, ERR_CHECKSUM);
				return ERROR_UNKNOWN;
			}
			blockbuffer->flags |= BCF_USED;
			markBlock(afsbase, ah->volume, blockbuffer->blocknum, -1);
		}
	}
	validBitmap(afsbase, ah->volume);
	blockbuffer->flags &= ~BCF_USED;
	priorbuffer->flags &= ~BCF_USED;
	return 0;
}

/********************************************
 Name  : linkNewBlock
 Descr.: links a new entry into a directorylist
 Input : dir  - directory to link in
         file - file to link
 Output: the block which must be written to disk
         -> if hashtable of directory changed its the same
            pointer as arg1; otherwise a HASHCHAIN pointer
            changed so we dont have to write this block but
            another one
         0 if error
*********************************************/
struct BlockCache *linkNewBlock
	(
		struct afsbase *afsbase,
		struct Volume *volume,
		struct BlockCache *dir,
		struct BlockCache *file
	)
{
ULONG key; /* parent; */
UBYTE buffer[32];
STRPTR name;

	file->buffer[BLK_PARENT(volume)]=AROS_LONG2BE(dir->blocknum);
	D(bug("afs.handler: linkNewBlock: linking block %ld\n",file->blocknum));
	name=(STRPTR)((ULONG)file->buffer+(BLK_FILENAME_START(volume)*4));
	StrCpyFromBstr(name,buffer);
	key=getHashKey(buffer,volume->SizeBlock-56,volume->flags)+BLK_TABLE_START;
	/* sort in ascending order */
	if (
			(dir->buffer[key]) &&
			(AROS_BE2LONG(dir->buffer[key])<file->blocknum))
	{
		dir=getBlock(afsbase, volume, AROS_BE2LONG(dir->buffer[key]));
		if (!dir)
			return 0;
		key=BLK_HASHCHAIN(volume);
		while (
					(dir->buffer[key]) &&
					(AROS_BE2LONG(dir->buffer[key])<file->blocknum)
				)
		{
			dir=getBlock(afsbase, volume, AROS_BE2LONG(dir->buffer[key]));
			if (!dir)
				return 0;
		}
	}
	file->buffer[BLK_HASHCHAIN(volume)]=dir->buffer[key];
	dir->buffer[key]=AROS_LONG2BE(file->blocknum);
	file->buffer[BLK_CHECKSUM]=0;
	file->buffer[BLK_CHECKSUM]=
		AROS_LONG2BE(0-calcChkSum(volume->SizeBlock,file->buffer));
	dir->buffer[BLK_CHECKSUM]=0;
	dir->buffer[BLK_CHECKSUM]=
		AROS_LONG2BE(0-calcChkSum(volume->SizeBlock,dir->buffer));
	return dir;
}

/********************************************
 Name  : getDirBlockBuffer
 Descr.: returns cacheblock of the block the
         last component before "/" or ":" of
         name refers to or cacheblock of ah
         if there is no such component
 Input : afsbase   - 
         ah        - filehandle name is relative to
         name      - name of object
         entryname - will be filled with a copy
                     of the last component of
                     name
 Output: 0 for error (global error will be set);
         pointer to a struct BlockCache otherwise
*********************************************/
struct BlockCache *getDirBlockBuffer
	(
		struct afsbase *afsbase,
		struct AfsHandle *ah,
		STRPTR name,
		STRPTR entryname
	)
{
ULONG block,len;
STRPTR end;
UBYTE buffer[256];

	end=PathPart(name);
	CopyMem(name,buffer,end-name);
	buffer[end-name]=0;
	if (end[0]=='/')
		end++;
	len=StrLen(name)+name-end;
	CopyMem(end, entryname, len);	//skip backslash or colon
	entryname[len]=0;
	return findBlock(afsbase, ah,buffer,&block);
}

/********************************************
 Name  : rename
 Descr.: rename an object
 Input : afsbase -
         dirah   - filehandle names are relative to
         oname   - object to rename
         newname - new name of the object
 Output: 0 for success; error code otherwise
*********************************************/
ULONG rename
	(
		struct afsbase *afsbase,
		struct AfsHandle *dirah,
		STRPTR oname,
		STRPTR newname
	)
{
struct BlockCache *lastlink,*oldfile, *dirblock;
ULONG block,dirblocknum,lastblock;
UBYTE newentryname[34];

	D(bug("afs.handler: rename(%ld,%s,%s)\n",dirah->header_block,oname,newname));
	dirblock=getDirBlockBuffer(afsbase, dirah, newname, newentryname);
	if (!dirblock)
		return error;
	dirblocknum=dirblock->blocknum;
	D(bug("afs.handler:    dir is on block %ld\n",dirblocknum));
	if (getHeaderBlock(afsbase, dirah->volume,newentryname,dirblock,&block))
	{
		dirblock->flags &= ~BCF_USED;
		return ERROR_OBJECT_EXISTS;
	}
	oldfile=findBlock(afsbase, dirah, oname, &lastblock);
	if (!oldfile)
		return error;
	oldfile->flags |= BCF_USED;
	// do we move a directory?
	if (
			AROS_BE2LONG
				(
					oldfile->buffer
						[BLK_SECONDARY_TYPE(dirah->volume)]
				)==ST_USERDIR
		)
	{
		// is newdirblock child of olock&oname
		dirblock=getBlock(afsbase, dirah->volume,dirblocknum);
		if (!dirblock)
		{
			oldfile->flags &= ~BCF_USED;
			return ERROR_UNKNOWN;
		}
		while ((block=AROS_BE2LONG(dirblock->buffer[BLK_PARENT(dirah->volume)])))
		{
			if (block==oldfile->blocknum)
			{
				oldfile->flags &= ~BCF_USED;
				return ERROR_OBJECT_IN_USE;
			}
			dirblock=getBlock(afsbase, dirah->volume,block);
			if (!dirblock)
			{
				oldfile->flags &= ~BCF_USED;
				return ERROR_UNKNOWN;
			}
		}
	}
	lastlink=getBlock(afsbase, dirah->volume, lastblock);
	if (!lastlink)
	{
		oldfile->flags &= ~BCF_USED;
		return ERROR_UNKNOWN;
	}
	lastlink->flags |= BCF_USED;
	unLinkBlock(afsbase, dirah->volume,lastlink, oldfile);
	/* rename in same dir ? */
	if (lastlink->blocknum==dirblocknum)
	{
	/* use same buffers! */
		dirblock=lastlink;
	}
 	/* otherwise we use different blocks */
	else
	{
		dirblock=getBlock(afsbase, dirah->volume, dirblocknum);
		if (!dirblock)
		{
			oldfile->flags &= ~BCF_USED;
			lastlink->flags &= ~BCF_USED;
			return ERROR_UNKNOWN;
		}
	}
	if (
			(
				AROS_BE2LONG
					(
						dirblock->buffer
							[
								BLK_SECONDARY_TYPE(dirah->volume)
							]
					)!=ST_USERDIR
			) &&
			(
				AROS_BE2LONG
					(
						dirblock->buffer
							[
								BLK_SECONDARY_TYPE(dirah->volume)
							]
					)!=ST_ROOT
			)
		)
	{
		oldfile->flags &= ~BCF_USED;
		lastlink->flags &= ~BCF_USED;
		return ERROR_OBJECT_WRONG_TYPE;
	}
	StrCpyToBstr
		(
			newentryname,
			(APTR)((ULONG)oldfile->buffer+(BLK_FILENAME_START(dirah->volume)*4))
		);
	dirblock=linkNewBlock(afsbase, dirah->volume,dirblock,oldfile);
	if (!dirblock)
	{
		oldfile->flags &= ~BCF_USED;
		lastlink->flags &= ~BCF_USED;
		return ERROR_UNKNOWN;
	}
	/* concurrent access if newdir=rootblock! */
	dirblock->flags |= BCF_USED;
	/*
		mark it as used, so that this buffer isnt
		used in invalidating volume!
	*/
	if (!setBitmapFlag(afsbase, dirah->volume, 0))
	{
		oldfile->flags &= ~BCF_USED;
		lastlink->flags &= ~BCF_USED;
		dirblock->flags &= ~BCF_USED;
		return ERROR_UNKNOWN;
	}
	/* now we can release that buffer */
	dirblock->flags &= ~BCF_USED;
	/*
		syscrash after this: 2 dirs pointing to the same
		dir and one wrong linked entry->recoverable
	*/
	writeBlock(afsbase, dirah->volume, dirblock);
	/*
		syscrash after this: directory pointing is now
		correct but one wrong linked entry->recoverable
	*/
	writeBlock(afsbase, dirah->volume, lastlink);
	writeBlock(afsbase, dirah->volume, oldfile);
	oldfile->flags &= ~BCF_USED;
	lastlink->flags &= ~BCF_USED;
	/*
		if newdir=rootblock we now write the correct
		(changed) buffer back
	*/
	setBitmapFlag(afsbase, dirah->volume, -1);
	return 0;
}

/********************************************
 Name  : createNewEntry
 Descr.: create a new object on disk
 Input : afsbase    -
         volume     -
         entrytype  - ST_USERDIR/ST_FILE/...
         entryname  - name of object
         dirblock   - pointer to struct BlockCache
                      containing a directory in which
                      name shall be created in
         protection - protection bit mask
 Output: 0 for error (global error set);
         pointer to struct BlockCache of the newly
         created object otherwise
*********************************************/
struct BlockCache *createNewEntry
	(
		struct afsbase *afsbase,
		struct Volume *volume,
		ULONG entrytype,
		STRPTR entryname,
		struct BlockCache *dirblock,
		ULONG protection
	)
{
struct BlockCache *newblock;
struct DateStamp ds;
ULONG i;

	dirblock->flags |= BCF_USED;
	if (getHeaderBlock(afsbase, volume, entryname, dirblock, &i))
	{
		dirblock->flags &= ~BCF_USED;
		error = ERROR_OBJECT_EXISTS;
		return 0;
	}
	error = 0;
	if (!invalidBitmap(afsbase, volume))
	{
		dirblock->flags &= ~BCF_USED;
		return 0;
	}
	i=allocBlock(afsbase, volume);
	if (i==0)
	{
		dirblock->flags &= ~BCF_USED;
		validBitmap(afsbase, volume);
		error=ERROR_DISK_FULL;
		return 0;
	}
	newblock=getFreeCacheBlock(afsbase, volume,i);
	if (!newblock)
	{
		dirblock->flags &= ~BCF_USED;
		validBitmap(afsbase, volume);
		error=ERROR_UNKNOWN;
		return 0;
	}
	newblock->flags |= BCF_USED;
	newblock->buffer[BLK_PRIMARY_TYPE]=AROS_LONG2BE(T_SHORT);
	for (i=BLK_BLOCK_COUNT;i<=BLK_COMMENT_END(volume);i++)
		newblock->buffer[i]=0;
	newblock->buffer[BLK_PROTECT(volume)]=AROS_LONG2BE(protection);
	DateStamp(&ds);
	newblock->buffer[BLK_DAYS(volume)]=AROS_LONG2BE(ds.ds_Days);
	newblock->buffer[BLK_MINS(volume)]=AROS_LONG2BE(ds.ds_Minute);
	newblock->buffer[BLK_TICKS(volume)]=AROS_LONG2BE(ds.ds_Tick);
	StrCpyToBstr
		(
			entryname,
			(APTR)((ULONG)newblock->buffer+(BLK_FILENAME_START(volume)*4))
		);
	for (i=BLK_FILENAME_END(volume)+1;i<BLK_HASHCHAIN(volume);i++)
		newblock->buffer[i]=0;
	newblock->buffer[BLK_PARENT(volume)]=AROS_LONG2BE(dirblock->blocknum);
	newblock->buffer[BLK_EXTENSION(volume)]=0;
	newblock->buffer[BLK_SECONDARY_TYPE(volume)]=AROS_LONG2BE(entrytype);
	newblock->buffer[BLK_OWN_KEY]=AROS_LONG2BE(newblock->blocknum);
	dirblock->flags &= ~BCF_USED;
	dirblock=linkNewBlock(afsbase, volume,dirblock,newblock);
	if (!dirblock)
	{
		markBlock(afsbase, volume,newblock->blocknum,-1);
		newblock->flags &= ~BCF_USED;
		newblock->acc_count=0;
		newblock->volume=0;
		validBitmap(afsbase, volume);
		return 0;
	}
	/*
		if crash after this block not yet linked->block
		not written to disk, bitmap corrected
	*/
	writeBlock(afsbase, volume, newblock);
	/* consistent after this */
	writeBlock(afsbase, volume, dirblock);
	validBitmap(afsbase, volume);
	newblock->flags &= ~BCF_USED;
	return newblock;
}


/********************************************
 Name  : createDir
 Descr.: create a directory object
 Input : afsbase    - 
         dirah      - filehandle filename is relative to
         filename   - path to the new directory
         protection - protection bit mask
 Output: pointer to struct AfsHandle;
         0 otherwise (global error set)
*********************************************/
struct AfsHandle *createDir
	(
		struct afsbase *afsbase,
		struct AfsHandle *dirah,
		STRPTR filename,
		ULONG protection
	)
{
struct AfsHandle *ah=0;
struct BlockCache *dirblock;
char dirname[34];

	D(bug("afs.handler: createDir(ah,%s,%ld)\n",filename,protection));
	if ((dirblock=getDirBlockBuffer(afsbase, dirah, filename, dirname)))
	{
		D(bug("afs.handler:    dir is on block %ld\n",dirblock->blocknum));
		dirblock=createNewEntry
			(
				afsbase,
				dirah->volume,
				ST_USERDIR,
				dirname,
				dirblock,
				protection
			);
		if (dirblock)
			ah=getHandle(afsbase, dirah->volume,dirblock, FMF_READ);
	}
	return ah;
}

