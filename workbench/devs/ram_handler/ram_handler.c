/*
    Copyright � 1995-2008, The AROS Development Team. All rights reserved.
    $Id$

    RAM: handler.
*/

#define  DEBUG 0
#include <aros/debug.h>

#include <exec/errors.h>
#include <exec/types.h>
#include <exec/resident.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <utility/tagitem.h>
#include <utility/utility.h>
#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/alib.h>
#include <proto/battclock.h>
#include <proto/dos.h>
#include <resources/battclock.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dosasl.h>
#include <dos/exall.h>
#include <dos/filesystem.h>
#include <aros/libcall.h>
#include <aros/symbolsets.h>

#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#include "ram_handler_gcc.h"
#endif
#include <stddef.h>
#include <string.h>

#include  "HashTable.h"

static void deventry();

#include LC_LIBDEFS_FILE

#define  NRF_NOT_REPLIED  NRF_MAGIC


/* Device node */
struct cnode
{
    struct MinNode node;
    LONG type;			/* ST_LINKDIR */
    STRPTR name; 		/* Link's name */
    struct cnode *self; 	/* Pointer to top of structure */
    struct hnode *link; 	/* NULL */
    LONG usecount;		/* >0 usecount locked:+(~0ul/2+1) */
    ULONG protect;		/* 0 */
    STRPTR comment;		/* NULL */
    struct vnode *volume;	/* Pointer to volume */
    struct DosList *doslist;	/* Pointer to doslist entry */
};

/* Volume node */
struct vnode
{
    struct MinNode node;
    LONG type;			/* ST_USERDIR */
    STRPTR name; 		/* Directory name */
    struct vnode *self; 	/* Points to top of structure */
    struct hnode *link; 	/* This one is linked to me */
    LONG usecount;		/* >0 usecount locked:+(~0ul/2+1) */
    ULONG protect;		/* 0 */
    STRPTR comment;		/* NULL */
    struct MinList receivers;
    struct DateStamp date;
    struct MinList list;	/* Contents of directory */
    ULONG volcount;		/* number of handles on this volume */
    struct DosList *doslist;	/* Pointer to doslist entry */
};

/* Directory node */
struct dnode
{
    struct MinNode node;
    LONG type;			/* ST_USERDIR */
    STRPTR name; 		/* Directory name */
    struct vnode *volume;	/* Volume's root directory */
    struct hnode *link; 	/* This one is linked to me */
    LONG usecount;		/* >0 usecount locked:+(~0ul/2+1) */
    ULONG protect;		/* protection bits */
    STRPTR comment;		/* Some comment */
    struct MinList receivers;
    struct DateStamp date;
    struct MinList list;	/* Contents of directory */
};

/* File node */
struct fnode
{
    struct MinNode node;
    LONG type;			/* ST_FILE */
    STRPTR name; 		/* Filename */
    struct vnode *volume;	/* Volume's root directory */
    struct hnode *link; 	/* This one is linked to me */
    LONG usecount;		/* >0 usecount locked:+(~0ul/2+1) */
    ULONG protect;		/* protection bits */
    STRPTR comment;		/* Some file comment */
    struct MinList receivers;
    struct DateStamp date;
    ULONG          flags;
    LONG size;			/* Filesize */
    UBYTE *blocks[16];		/* Upto 0x1000 bytes */
    UBYTE **iblocks[4]; 	/* Upto 0x41000 bytes */
    UBYTE ***i2block;		/* Upto 0x1041000 bytes */
    UBYTE ****i3block;		/* Upto 0x101041000 bytes */
};

/* Softlink node */
struct snode
{
    struct MinNode node;
    LONG type;			/* ST_SOFTLINK */
    STRPTR name; 		/* Link's name */
    struct vnode *volume;	/* Volume's root directory */
    struct hnode *link; 	/* This one is hardlinked to me */
    LONG usecount;		/* >0 usecount locked:+(~0ul/2+1) */
    ULONG protect;		/* protection bits */
    STRPTR comment;		/* Some file comment */
    char *contents;		/* Contents of soft link */
};

/* Hardlink node */
struct hnode
{
    struct MinNode node;
    LONG type;			/* ST_LINKDIR */
    STRPTR name; 		/* Link's name */
    struct vnode *volume;	/* Volume's root directory */
    struct hnode *link; 	/* This one is hardlinked to me */
    LONG usecount;		/* >0 usecount locked:+(~0ul/2+1) */
    ULONG protect;		/* protection bits */
    STRPTR comment;		/* Some file comment */
    struct hnode *orig; 	/* original object */
};

/*****************************************************************************/

#define  FILEFLAGS_Changed  1

static STRPTR getName(struct rambase *rambase, struct dnode *dn);

typedef enum
{
    NOTIFY_Add,
    NOTIFY_Delete,
    NOTIFY_Write,
    NOTIFY_Close,
    NOTIFY_Open,
} NType;

void Notify_notifyTasks(struct rambase *rambase,
			struct MinList *notifications);

BOOL Notify_initialize(struct rambase *rambase);
void Notify_fileChange(struct rambase *rambase, NType type,
		       struct fnode *file);
void Notify_directoryChange(struct rambase *rambase, NType type,
			    struct dnode *dir);


BOOL Notify_addNotification(struct rambase *rambase, struct dnode *dn,
			    struct NotifyRequest *nr);
void Notify_removeNotification(struct rambase *rambase,
			       struct NotifyRequest *nr);

/****************************************************************************/


#define BLOCKSIZE	256
#define PBLOCKSIZE	(256*sizeof(UBYTE *))

struct filehandle
{
    struct dnode *node;
    IPTR position;
};

static int OpenDev(LIBBASETYPEPTR rambase, struct IOFileSys *iofs);

static int GM_UNIQUENAME(Init)(LIBBASETYPEPTR rambase)
{
    /* This function is single-threaded by exec by calling Forbid. */

    struct MsgPort *port;
    struct Task *task;
    struct SignalSemaphore *semaphore;
    APTR stack;

    port = (struct MsgPort *)AllocMem(sizeof(struct MsgPort),
                                      MEMF_PUBLIC | MEMF_CLEAR
    );
    if (port != NULL)
    {
        rambase->port = port;
        NewList(&port->mp_MsgList);
        port->mp_Node.ln_Type = NT_MSGPORT;
        port->mp_SigBit = SIGB_SINGLE;

        task = (struct Task *)AllocMem(sizeof(struct Task),
                                       MEMF_PUBLIC | MEMF_CLEAR
        );

        if (task != NULL)
        {
            port->mp_SigTask = task;
            port->mp_Flags = PA_IGNORE;
            NewList(&task->tc_MemEntry);
            task->tc_Node.ln_Type = NT_TASK;
            task->tc_Node.ln_Name = "ram.handler task";

            stack = AllocMem(AROS_STACKSIZE, MEMF_PUBLIC);

            if (stack != NULL)
            {
                struct TagItem tasktags[] =
                {
                    {TASKTAG_ARG1, (IPTR)rambase},
                    {TAG_DONE	    	    	}
                };
			
                task->tc_SPLower = stack;
                task->tc_SPUpper = (BYTE *)stack + AROS_STACKSIZE;
#if AROS_STACK_GROWS_DOWNWARDS
                task->tc_SPReg = (BYTE *)task->tc_SPUpper - SP_OFFSET;
#else
                task->tc_SPReg = (BYTE *)task->tc_SPLower + SP_OFFSET;
#endif

                semaphore = (struct SignalSemaphore *)AllocMem(sizeof(struct SignalSemaphore),
                                                               MEMF_PUBLIC | MEMF_CLEAR
                );

                if (semaphore != NULL)
                {
                    rambase->sigsem = semaphore;
                    InitSemaphore(semaphore);

                    /*
                     * Modules compiled with noexpunge always have seglist == NULL
                     * The seglist is not kept as it is not needed because the module never
                     * can be expunged
                    if (rambase->seglist==NULL) /* Are we a ROM module? * /
                     */
                    {
                        struct DeviceNode *dn;
                        /* Install RAM: handler into device list
                         *
                         * KLUDGE: ram.handler should create only one device node, depending on
                         * the startup packet it gets. The mountlists for RAM: should be into dos.library bootstrap
                         * routines.
                         */

                        if((dn = AllocMem(sizeof (struct DeviceNode) + 4 + 3 + 2, MEMF_CLEAR|MEMF_PUBLIC)))
                        {
                            struct IOFileSys dummyiofs;

                            if (OpenDev(rambase, &dummyiofs))
                            {
                                BSTR s = MKBADDR(((IPTR)dn + sizeof(struct DeviceNode) + 3) & ~3);

                                ((struct Library *)rambase)->lib_OpenCnt++;
					
                                AROS_BSTR_putchar(s, 0, 'R');
                                AROS_BSTR_putchar(s, 1, 'A');
                                AROS_BSTR_putchar(s, 2, 'M');
                                AROS_BSTR_setstrlen(s, 3);

                                dn->dn_Type	    = DLT_DEVICE;
                                dn->dn_Ext.dn_AROS.dn_Unit	    = dummyiofs.IOFS.io_Unit;
                                dn->dn_Ext.dn_AROS.dn_Device   = dummyiofs.IOFS.io_Device;
                                dn->dn_Handler  = NULL;
                                dn->dn_Startup  = NULL;
                                dn->dn_Name  = s;
                                dn->dn_Ext.dn_AROS.dn_DevName  = AROS_BSTR_ADDR(dn->dn_Name);

                                if (AddDosEntry((struct DosList *)dn))
                                    if (NewAddTask(task, deventry, NULL, tasktags) != NULL)
                                        return TRUE;
                            }

                            FreeMem(dn, sizeof (struct DeviceNode));
                        }
                    }
/*                    else
                        if (NewAddTask(task, deventry, NULL, tasktags) != NULL)
                            return TRUE;
*/
                    FreeMem(semaphore, sizeof(struct SignalSemaphore));
                }

                FreeMem(stack, AROS_STACKSIZE);
            }

            FreeMem(task, sizeof(struct Task));
        }

        FreeMem(port, sizeof(struct MsgPort));
    }

    return FALSE;
}


/***************************************************************************/

void nullDelete(struct rambase *rambase, void *key, struct List *list)
{
    // struct Receiver *rr;
    // struct Node     *tempNode;
    //
    //    ForeachNodeSafe(list, rr, tempNode)
    //    {
    //        Remove(&rr->node);
    //        FreeVec(rr);
    //    }

    FreeVec(key);

    /* The list is part of the HashNode so we shouldn't free that */
}


ULONG stringHash(struct rambase *rambase, const void *key)
{
    STRPTR str = (STRPTR)key;
    ULONG  val = 1;

    while (*str != 0)
    {
	val *= ToLower(*str++);
	val <<= 2;
    }

    return val;
}

static int
my_strcasecmp(struct rambase *rambase, const void *s1, const void *s2)
{
    return strcasecmp(s1, s2);
}

/****************************************************************************/


STRPTR Strdup(struct rambase *rambase, CONST_STRPTR string)
{
    CONST_STRPTR s2 = string;
    STRPTR s3;

    while(*s2++)
	;

    s3 = (STRPTR)AllocVec(s2 - string, MEMF_ANY);

    if (s3 != NULL)
    {
	CopyMem(string, s3, s2 - string);
    }

    return s3;
}


void Strfree(struct rambase *rambase, STRPTR string)
{
    FreeVec(string);
}


static int GM_UNIQUENAME(Open)
(
    LIBBASETYPEPTR rambase,
    struct IOFileSys *iofs,
    ULONG unitnum,
    ULONG flags
)
{
    /* Mark Message as recently used. */
    iofs->IOFS.io_Message.mn_Node.ln_Type = NT_REPLYMSG;

    return OpenDev(rambase, iofs);
}

static void GetCurrentDate(LIBBASETYPEPTR rambase, struct DateStamp *date)
{
    APTR BattClockBase = NULL;
    ULONG secs = 0;

    DateStamp(date);
    if (date->ds_Days != 0)
	return;

    BattClockBase = OpenResource(BATTCLOCKNAME);
    if (BattClockBase)
	secs = ReadBattClock();

    date->ds_Days = secs / (60 * 60 * 24);
    secs %= (60 * 60 * 24);
    date->ds_Minute = secs / 60;
    date->ds_Tick = 0;
}

static int OpenDev(LIBBASETYPEPTR rambase, struct IOFileSys *iofs)
{
    struct filehandle *fhv, *fhc;
    struct vnode *vol;
    struct cnode *dev;
    struct DeviceList *dlv;
    
    iofs->IOFS.io_Error = ERROR_NO_FREE_STORE;

    fhv = (struct filehandle*)AllocMem(sizeof(struct filehandle), MEMF_CLEAR);

    if (fhv != NULL)
    {
	fhc = (struct filehandle*)AllocMem(sizeof(struct filehandle),
					   MEMF_CLEAR);

	if (fhc != NULL)
        {
	    vol = (struct vnode *)AllocMem(sizeof(struct vnode), MEMF_CLEAR);

	    if (vol != NULL)
	    {
		dev = (struct cnode *)AllocMem(sizeof(struct cnode),
					       MEMF_CLEAR);
		GetCurrentDate(rambase, &vol->date);

		if (dev != NULL)
		{
		    vol->name = Strdup(rambase, "Ram Disk");

		    if (vol->name != NULL)
		    {
			dlv = (struct DeviceList *)MakeDosEntry("Ram Disk",
								DLT_VOLUME);

			if (dlv != NULL)
			{
			    vol->type = ST_USERDIR;
			    vol->self = vol;
			    vol->doslist = (struct DosList *)dlv;
			    vol->protect = 0UL;
			    NewList((struct List *)&vol->list);
			    NewList((struct List *)&vol->receivers);
			    fhv->node = (struct dnode *)vol;
			    dlv->dl_Ext.dl_AROS.dol_Unit = (struct Unit *)fhv;
			    dlv->dl_Ext.dl_AROS.dol_Device = &rambase->device;
			    dlv->dl_VolumeDate = vol->date;
			    dev->type = ST_LINKDIR;
			    dev->self = dev;
			    dev->volume=vol;
			    fhc->node = (struct dnode *)dev;
			    iofs->IOFS.io_Unit = (struct Unit *)fhc;
			    iofs->IOFS.io_Device = &rambase->device;
			    AddDosEntry((struct DosList *)dlv);
			    rambase->root = vol;
			    iofs->IOFS.io_Error = 0;

			    rambase->notifications =
				HashTable_new(rambase, 100, stringHash,
					      my_strcasecmp, nullDelete);

			    return TRUE;
			}

			Strfree(rambase, vol->name);
		    }

		    FreeMem(dev, sizeof(struct cnode));
		}

		FreeMem(vol, sizeof(struct vnode));
	    }

	    FreeMem(fhc, sizeof(struct filehandle));
	}

	FreeMem(fhv, sizeof(struct filehandle));
    }
    
    iofs->IOFS.io_Error = IOERR_OPENFAIL;

    return FALSE;
}

static int GM_UNIQUENAME(Expunge)(LIBBASETYPEPTR rambase)
{
    /*
	This function is single-threaded by exec by calling Forbid.
	Never break the Forbid() or strange things might happen.
    */

    /* Kill device task and free all resources */
    RemTask(rambase->port->mp_SigTask);
    FreeMem(rambase->sigsem, sizeof(struct SignalSemaphore));
    FreeMem(((struct Task *)rambase->port->mp_SigTask)->tc_SPLower,
	    AROS_STACKSIZE);
    FreeMem(rambase->port->mp_SigTask, sizeof(struct Task));
    FreeMem(rambase->port, sizeof(struct MsgPort));
    
    return TRUE;
}


AROS_LH1(void, beginio,
 AROS_LHA(struct IOFileSys *, iofs, A1),
	   struct rambase *, rambase, 5, Ram)
{
    AROS_LIBFUNC_INIT

    /* WaitIO will look into this */
    iofs->IOFS.io_Message.mn_Node.ln_Type = NT_MESSAGE;

    /* Nothing is done quick */
    iofs->IOFS.io_Flags &= ~IOF_QUICK;

    /* So let the device task do it */
    PutMsg(rambase->port, &iofs->IOFS.io_Message);

    AROS_LIBFUNC_EXIT
}


AROS_LH1(LONG, abortio,
 AROS_LHA(struct IOFileSys *, iofs, A1),
	   struct rambase *, rambase, 6, Ram)
{
    AROS_LIBFUNC_INIT
    return 0;
    AROS_LIBFUNC_EXIT
}


static LONG getblock(struct rambase *rambase, struct fnode *file, LONG block,
		     int mode, UBYTE **result)
{
    ULONG a, i;
    UBYTE **p, **p2;

    if (block < 0x10)
    {
	p = &file->blocks[block];
	block= 0 ;
	i = 0;
    }
    else if (block < 0x410)
    {
	block -= 0x10;
	p = (UBYTE **)&file->iblocks[block/0x100];
	block &= 0xff;
	i = 1;
    }
    else if(block < 0x10410)
    {
	block -= 0x410;
	p = (UBYTE **)&file->i2block;
	i = 2;
    }
    else
    {
	block -= 0x10410;
	p = (UBYTE **)&file->i3block;
	i = 3;
    }

    switch (mode)
    {
    case -1:
	p2 = (UBYTE **)*p;

	if (!block)
	{
	    *p = NULL;
	}

	p = p2;

	while (i-- && p != NULL)
	{
	    a = (block >> i*8) & 0xff;
	    p2 = (UBYTE **)p[a];

	    if (!(block & ((1 << i*8) - 1)))
	    {
		p[a] = NULL;

		if (!a)
		{
		    FreeMem(p, PBLOCKSIZE);
		}
	    }

	    p = p2;
	}

	if (p != NULL)
	{
	    FreeMem(p, BLOCKSIZE);
	}
	
	break;
	
    case 0:
	p = (UBYTE **)*p;

	while (i-- && p != NULL)
	{
	    p = ((UBYTE ***)p)[(block >> i*8) & 0xff];
	}

	*result = (UBYTE *)p;
	break;

    case 1:
	while (i--)
	{
	    if (*p == NULL)
	    {
		*p= AllocMem(PBLOCKSIZE, MEMF_CLEAR);

		if (*p == NULL)
		{
		    return ERROR_NO_FREE_STORE;
		}
	    }

	    p = (UBYTE **)*p + ((block >> i*8) & 0xff);
	}

	if (*p == NULL)
	{
	    *p = AllocMem(BLOCKSIZE, MEMF_CLEAR);

	    if (*p == NULL)
	    {
		return ERROR_NO_FREE_STORE;
	    }
	}

	*result = *p;
	break;
    } /* switch (mode) */

    return 0;
}


static void zerofill(UBYTE *address, ULONG size)
{
    while (size--)
    {
	*address++ = 0;
    }
}


static void shrinkfile(struct rambase *rambase, struct fnode *file, LONG size)
{
    ULONG blocks, block;
    UBYTE *p;
    
    blocks = (size + BLOCKSIZE - 1)/BLOCKSIZE;
    block  = (file->size + BLOCKSIZE - 1)/BLOCKSIZE;

    for(;block-- > blocks;)
    {
	(void)getblock(rambase, file, block, -1, &p);
    }

    if (size & 0xff)
    {
	(void)getblock(rambase, file, size, 0, &p);

	if (p != NULL)
	{
	    zerofill(p + (size & 0xff), -size & 0xff);
	}
    }

    file->size = size;
}


static void delete(struct rambase *rambase, struct fnode *file)
{
    struct hnode *link, *new, *more;
    struct Node *node;

    Strfree(rambase, file->name);
    Strfree(rambase, file->comment);
    Remove((struct Node *)file);

    if (file->type == ST_LINKDIR)
    {
	/* It is a link. Remove it from the chain. */

	link = ((struct hnode *)file)->orig;
	((struct hnode *)file)->orig = NULL;
	file->type = link->type;

	while((struct fnode *)link->link != file)
	{
	    link = link->link;
	}

	link->link = file->link;
    }
    else if (file->link != NULL)
    {
	/* If there is a hard link to the object make the link the original */

	link = file->link;
	link->type = file->type;
	more = new->link;

	while (more != NULL)
	{
	    more->orig = new;
	    more = more->link;
	}

	switch (file->type)
	{
	case ST_USERDIR:
	    while((node = RemHead((struct List *)&((struct dnode *)file)->list)) != NULL)
		AddTail((struct List *)&((struct dnode *)file)->list, node);
	    break;

	case ST_FILE:
	    CopyMemQuick(&file->size, &((struct fnode *)new)->size,
			 sizeof(struct fnode) - offsetof(struct fnode, size));
	    zerofill((UBYTE *)&file->size,
		     sizeof(struct fnode) - offsetof(struct fnode, size));
	    break;

	case ST_SOFTLINK:
		((struct snode *)new)->contents = ((struct snode *)file)->contents;
		((struct snode *)file)->contents = NULL;
		break;
	}
    } /* else if (file->link != NULL) */

    switch (file->type)
    {
    case ST_USERDIR:
	Notify_directoryChange(rambase, NOTIFY_Delete, (struct dnode *)file);

	FreeMem(file, sizeof(struct dnode));

	return;

    case ST_FILE:
	Notify_fileChange(rambase, NOTIFY_Delete, file);

	shrinkfile(rambase, file, 0);
	FreeMem(file, sizeof(struct fnode));

	return;
	
    case ST_SOFTLINK:
	Strfree(rambase, ((struct snode *)file)->contents);
	FreeMem(file, sizeof(struct snode));

	return;
    }
}


static int fstrcmp(struct rambase *rambase, CONST_STRPTR s1, CONST_STRPTR s2)
{
    for (;;)
    {
	if (ToLower(*s1) != ToLower(*s2))
	{
	    return *s1 || *s2 != '/';
	}

	if (!*s1)
	{
	    return 0;
	}

	s1++;
	s2++;
    }
}


/* Find a node for a given name */
static LONG findname(struct rambase *rambase, CONST_STRPTR *name,
		     struct dnode **dnode)
{
    struct dnode *cur = *dnode;
    CONST_STRPTR rest = *name;

    for (;;)
    {
	if (cur->type == ST_LINKDIR)
	{
	    cur = (struct dnode *)((struct hnode *)cur)->orig;
	}

	if (!*rest)
	{
	    break;
	}

	if (*rest == '/')
	{
	    if ((struct dnode *)cur->volume == cur)
	    {
		return ERROR_OBJECT_NOT_FOUND;
	    }
	    
	    while (cur->node.mln_Pred != NULL)
	    {
		cur = (struct dnode *)cur->node.mln_Pred;
	    }

	    cur = (struct dnode *)((BYTE *)cur - offsetof(struct dnode, list));
	}
	else
	{
	    if (cur->type == ST_SOFTLINK)
	    {
		*dnode = cur;
		*name = rest;

		return ERROR_IS_SOFT_LINK;
	    }
	    
	    if (cur->type != ST_USERDIR)
	    {
		return ERROR_DIR_NOT_FOUND;
	    }

	    *dnode = cur;
	    cur = (struct dnode *)cur->list.mlh_Head;

	    for (;;)
	    {
		if (cur->node.mln_Succ == NULL)
		{
		    *name = rest;
		    
		    return ERROR_OBJECT_NOT_FOUND;
		}

		if (!fstrcmp(rambase, cur->name, rest))
		{
		    break;
		}

		cur = (struct dnode *)cur->node.mln_Succ;
	    }
	}
	
	while (*rest)
	{
	    if (*rest++ == '/')
	    {
		break;
	    }
	}
    }

    *dnode = cur;

    return 0;
}


static LONG set_file_size(struct rambase *rambase, struct filehandle *handle,
			  QUAD *offp, LONG mode)
{
    struct fnode *file = (struct fnode *)handle->node;
    QUAD size = *offp;

    if (file->type != ST_FILE)
    {
	return ERROR_OBJECT_WRONG_TYPE;
    }

    switch(mode)
    {
    case OFFSET_BEGINNING:
	break;

    case OFFSET_CURRENT:
	size += handle->position;
	break;

    case OFFSET_END:
	size += file->size;
	break;

    default:
	return ERROR_NOT_IMPLEMENTED;
    }

    if (size < 0)
    {
	return ERROR_SEEK_ERROR;
    }
    
    if (size < file->size)
    {
	shrinkfile(rambase, file, size);
    }

    file->size = *offp = size;

    return 0;
}


static LONG read(struct rambase *rambase, struct filehandle *handle,
		 APTR buffer, LONG *numbytes)
{
    struct fnode *file = (struct fnode *)handle->node;
    ULONG num = *numbytes;
    ULONG size = file->size;
    ULONG block, offset;
    UBYTE *buf = buffer, *p;

    if (handle->position >= size)
    {
	num = 0;
    }
    else if (handle->position + num > size)
    {
	num = size - handle->position;
    }

    block  = handle->position/BLOCKSIZE;
    offset = handle->position & (BLOCKSIZE - 1);
    size   = BLOCKSIZE - offset;

    while (num)
    {
	if (size > num)
	{
	    size = num;
	}

	(void)getblock(rambase, file, block, 0, &p);

	if (p != NULL)
	{
	    CopyMem(p + offset, buffer, size);
	}
	else
	{
	    zerofill(buffer, size);
	}

	buffer += size;
	num -= size;
	block++;
	offset = 0;
	size = BLOCKSIZE;
    }

    *numbytes = (UBYTE *)buffer - buf;
    handle->position += *numbytes;

    return 0;
}


static LONG write(struct rambase *rambase, struct filehandle *handle,
		  UBYTE *buffer, LONG *numbytes)
{
    struct fnode *file = (struct fnode *)handle->node;
    ULONG num = *numbytes;
    ULONG size = file->size;
    ULONG block, offset;
    UBYTE *buf = buffer, *p;
    LONG error = 0;

    if ((LONG)(handle->position + num) < 0)
    {
	return ERROR_OBJECT_TOO_LARGE;
    }

    Notify_fileChange(rambase, NOTIFY_Write, file);

    block = handle->position/BLOCKSIZE;
    offset = handle->position & (BLOCKSIZE - 1);
    size = BLOCKSIZE - offset;

    while (num)
    {
	if (size > num)
	{
	    size = num;
	}

	error = getblock(rambase, file, block, 1, &p);

	if (error)
	{
	    break;
	}

	CopyMem(buffer, p + offset, size);
	buffer += size;
	num -= size;
	block++;
	offset = 0;
	size = BLOCKSIZE;
    }

    *numbytes = (UBYTE *)buffer - buf;
    handle->position += *numbytes;
    DateStamp(&file->date);

    if (handle->position > file->size)
    {
	file->size = handle->position;
    }

    return error;
}


static LONG lock(struct dnode *dir, ULONG mode)
{
    if ((mode & FMF_EXECUTE) && (dir->protect & FMF_EXECUTE))
    {
	return ERROR_NOT_EXECUTABLE;
    }

    if ((mode & FMF_WRITE) && (dir->protect & FMF_WRITE))
    {
	return ERROR_WRITE_PROTECTED;
    }

    if ((mode & FMF_READ) && (dir->protect & FMF_READ))
    {
	return ERROR_READ_PROTECTED;
    }

    if (mode & FMF_LOCK)
    {
	if (dir->usecount)
	{
	    return ERROR_OBJECT_IN_USE;
	}
	
	dir->usecount = ((ULONG)~0)/2 + 1;
    }
    else
    {
	if (dir->usecount < 0)
	{
	    return ERROR_OBJECT_IN_USE;
	}
    }

    dir->usecount++;
    dir->volume->volcount++;

    return 0;
}


static LONG open_(struct rambase *rambase, struct filehandle **handle,
		  CONST_STRPTR name, ULONG mode)
{
    struct dnode *dir = (*handle)->node;
    struct filehandle *fh;
    LONG error;

    fh = (struct filehandle *)AllocMem(sizeof(struct filehandle), MEMF_CLEAR);

    if (fh != NULL)
    {
	error = findname(rambase, &name, &dir);

	if (!error)
	{
	    error = lock(dir, mode);

	    if (!error)
	    {
		fh->node = dir;
		*handle = fh;

		if (dir->type == ST_FILE)
		{
		    Notify_fileChange(rambase, NOTIFY_Open,
		    		      (struct fnode *)dir);
		}
		else if (dir->type == ST_USERDIR)
		{
		    D(kprintf("Notifying OPEN at dir %s\n", dir->name));
		    Notify_directoryChange(rambase, NOTIFY_Open, dir);
		}

		return 0;
	    }
	}
	
	FreeMem(fh, sizeof(struct filehandle));
    }
    else
    {
	error = ERROR_NO_FREE_STORE;
    }

    return error;
}


static LONG open_file(struct rambase *rambase, struct filehandle **handle,
		      CONST_STRPTR name, ULONG mode, ULONG protect)
{
    struct dnode *dir = (*handle)->node;
    struct filehandle *fh;
    LONG error = 0;

    fh = AllocMem(sizeof(struct filehandle), MEMF_CLEAR);

    if (fh != NULL)
    {
	error = findname(rambase, &name, &dir);

	if ((mode & FMF_CREATE) && error == ERROR_OBJECT_NOT_FOUND)
	{
	    CONST_STRPTR s = name;
	    struct fnode *file;

	    while (*s)
	    {
		if (*s++ == '/')
		{
		    return error;
		}
	    }

	    file = (struct fnode *)AllocMem(sizeof(struct fnode), MEMF_CLEAR);
	    DateStamp(&file->date);

	    if (file != NULL)
	    {
		file->name = Strdup(rambase, name);

		if (file->name != NULL)
		{
		    file->type = ST_FILE;
		    file->protect = protect;
		    file->volume = dir->volume;
		    NewList((struct List *)&file->receivers);
		    AddTail((struct List *)&dir->list, (struct Node *)file);
		    error = lock((struct dnode *)file, mode);

		    if (!error)
		    {
			fh->node = (struct dnode *)file;
			*handle = fh;

			Notify_fileChange(rambase, NOTIFY_Add, file);

			return 0;
		    }
		    
		    Strfree(rambase, file->name);
		}

		FreeMem(file,sizeof(struct fnode));
	    }
	    
	    error = ERROR_NO_FREE_STORE;
	}
	else if (!error)
	{
	    if (dir->type != ST_FILE)
	    {
		error = ERROR_OBJECT_WRONG_TYPE;
	    }
	    else
	    {
		error = lock(dir, mode);

		if (!error)
		{
		    /* stegerg */
		    if (mode & FMF_CLEAR)
		    {
		    	shrinkfile(rambase, (struct fnode *)dir, 0);
			dir->protect = protect;
		    }
		    /* end stegerg */
		    
		    fh->node = dir;
		    *handle = fh;

		    return 0;
		}
	    }
	} /* else if (!error) */

	FreeMem(fh, sizeof(struct filehandle));
    }

    return error;
}


static LONG create_dir(struct rambase *rambase, struct filehandle **handle,
		       CONST_STRPTR name, ULONG protect)
{
    struct dnode *dir = (*handle)->node, *new;
    struct filehandle *fh;
    LONG error;

    if (dir->protect & FIBF_WRITE)
    {
        return ERROR_WRITE_PROTECTED;
    }

    error = findname(rambase, &name, &dir);

    if (!error)
    {
	return ERROR_OBJECT_EXISTS;
    }

    if (error != ERROR_OBJECT_NOT_FOUND)
    {
	return error;
    }

    if (strchr(name, '/') != NULL)
    {
	return error;
    }

    fh = AllocMem(sizeof(struct filehandle), MEMF_CLEAR);

    if (fh != NULL)
    {
	new = (struct dnode *)AllocMem(sizeof(struct dnode), MEMF_CLEAR);

	if (new != NULL)
	{
	    new->name = Strdup(rambase, name);

	    if (new->name != NULL)
	    {
		new->type = ST_USERDIR;
		new->protect = protect;
		new->volume = dir->volume;
		new->volume->volcount++;
		new->usecount = ((ULONG)~0)/2 + 2;
		NewList((struct List *)&new->list);
		NewList((struct List *)&new->receivers);
		AddTail((struct List *)&dir->list, (struct Node *)new);
		fh->node = new;
		*handle = fh;
		DateStamp(&new->date);

		D(bug("New (%p): Name = %s\n", new, new->name));
		Notify_directoryChange(rambase, NOTIFY_Add, new);

		return 0;
	    }

	    FreeMem(new, sizeof(struct dnode));
	}

	FreeMem(fh, sizeof(struct filehandle));
    }

    return ERROR_NO_FREE_STORE;
}


static LONG free_lock(struct rambase *rambase, struct filehandle *filehandle)
{
    struct dnode *dnode = filehandle->node;

    dnode->usecount = (dnode->usecount - 1) & ((ULONG)~0)/2;
    FreeMem(filehandle, sizeof(struct filehandle));
    dnode->volume->volcount--;

    return 0;
}


static LONG seek(struct rambase *rambase, struct filehandle *filehandle,
		 QUAD *posp, LONG mode)
{
    struct fnode *file = (struct fnode *)filehandle->node;
    QUAD pos = *posp;

    if (file->type != ST_FILE)
    {
	return ERROR_OBJECT_WRONG_TYPE;
    }

    switch (mode)
    {
    case OFFSET_BEGINNING:
	break;

    case OFFSET_CURRENT:
	pos += filehandle->position;
	break;

    case OFFSET_END:
	pos += file->size;
	break;

    default:
	return ERROR_NOT_IMPLEMENTED;
    }

    if (pos < 0)
    {
	return ERROR_SEEK_ERROR;
    }

    *posp = filehandle->position;
    filehandle->position = pos;

    return 0;
}


static const ULONG sizes[]=
{
    0,
    offsetof(struct ExAllData,ed_Type),
    offsetof(struct ExAllData,ed_Size),
    offsetof(struct ExAllData,ed_Prot),
    offsetof(struct ExAllData,ed_Days),
    offsetof(struct ExAllData,ed_Comment),
    offsetof(struct ExAllData,ed_OwnerUID),
    sizeof(struct ExAllData)
};


static LONG examine(struct fnode *file,
                    struct ExAllData *ead,
                    ULONG  size, 
                    ULONG  type,
                    LONG   *dirpos)
{
    STRPTR next, end, name;

    if (type > ED_OWNER)
    {
	return ERROR_BAD_NUMBER;
    }

    next = (STRPTR)ead + sizes[type];
    end = (STRPTR)ead + size;

    if(next>end) /* > is correct. Not >= */
	return ERROR_BUFFER_OVERFLOW;

    /* Use *dirpos to store information for ExNext()
     * *dirpos is copied to fib->fib_DiskKey in Examine()
     */ 
    if (file->type == ST_USERDIR)
    {
	*dirpos = (LONG)(((struct dnode*)file)->list.mlh_Head);
    }
    else
    {
	/* ExNext() should not be called in this case anyway */
	*dirpos = (LONG)file;
    }
    
    switch(type)
    {
    case ED_OWNER:
	ead->ed_OwnerUID = 0;
	ead->ed_OwnerGID = 0;

	/* Fall through */
    case ED_COMMENT:
	if (file->comment != NULL)
	{
	    ead->ed_Comment = next;
	    name = file->comment;

	    for (;;)
	    {
		if (next >= end)
		{
		    return ERROR_BUFFER_OVERFLOW;
		}

		if (!(*next++ = *name++))
		{
		    break;
		}
	    }
	}
	else
	{
	    ead->ed_Comment = NULL;
	}

	/* Fall through */
    case ED_DATE:
	ead->ed_Days = file->date.ds_Days;
	ead->ed_Mins = file->date.ds_Minute;
	ead->ed_Ticks = file->date.ds_Tick;

	/* Fall through */
    case ED_PROTECTION:
	ead->ed_Prot = file->protect;

	/* Fall through */
    case ED_SIZE:
	ead->ed_Size = file->size;

	/* Fall through */
    case ED_TYPE:
	ead->ed_Type = file->type;

	if (((struct vnode *)file)->self == (struct vnode *)file)
	{
	    ead->ed_Type=ST_ROOT;
	}

	/* Fall through */
    case ED_NAME:
	ead->ed_Name = next;
	name = file->name;

	for (;;)
	{
	    if (next >= end)
	    {
		return ERROR_BUFFER_OVERFLOW;
	    }

	    if (!(*next++ = *name++))
	    {
		break;
	    }
	}

	/* Fall through */
    case 0:
	ead->ed_Next = (struct ExAllData *)(((IPTR)next + AROS_PTRALIGN - 1) & ~(AROS_PTRALIGN - 1));
    }

    return 0;
}


static LONG examine_next(struct rambase *rambase,
    			 struct filehandle    *dir,
                         struct FileInfoBlock *FIB)
{
    struct fnode *file = (struct fnode *)FIB->fib_DiskKey;
    
    ASSERT_VALID_PTR_OR_NULL(file);
    
    if (file->node.mln_Succ == NULL)
    {
	return ERROR_NO_MORE_ENTRIES;
    }

    FIB->fib_OwnerUID	    = 0;
    FIB->fib_OwnerGID	    = 0;
    
    FIB->fib_Date.ds_Days   = file->date.ds_Days;
    FIB->fib_Date.ds_Minute = file->date.ds_Minute;
    FIB->fib_Date.ds_Tick   = file->date.ds_Tick;
    FIB->fib_Protection	    = file->protect;
    FIB->fib_Size	    = file->size;
    FIB->fib_DirEntryType   = file->type;

    strncpy(FIB->fib_FileName, file->name, MAXFILENAMELENGTH - 1);
    strncpy(FIB->fib_Comment, file->comment != NULL ? file->comment : "",
	    MAXCOMMENTLENGTH - 1);
    
    FIB->fib_DiskKey = (LONG)file->node.mln_Succ;

    return 0;
}


static LONG examine_all(struct filehandle *dir, 
                        struct ExAllData *ead, 
                        struct ExAllControl *eac, 
                        ULONG size, 
                        ULONG type)
{
    STRPTR end;
    struct ExAllData *last=NULL;
    struct fnode *ent;
    LONG error;
    LONG dummy; /* not anything is done with this value but passed to examine */
    end = (STRPTR)ead + size;

    eac->eac_Entries = 0;
    if (dir->node->type != ST_USERDIR)
    {
	return ERROR_OBJECT_WRONG_TYPE;
    }

    ent = (struct fnode *)eac->eac_LastKey;

    if (ent == NULL)
    {
	ent = (struct fnode *)dir->node->list.mlh_Head;
	if (ent->node.mln_Succ) ent->usecount++;
    }
 
    if (ent->node.mln_Succ == NULL)
    {
	return ERROR_NO_MORE_ENTRIES;
    }

    ent->usecount--;

    do
    {
	error = examine(ent, ead, end - (STRPTR)ead, type, &dummy);

	if (error == ERROR_BUFFER_OVERFLOW)
	{
	    if (last == NULL)
	    {
		return error;
	    }

	    ent->usecount++;
	    last->ed_Next = NULL;
	    eac->eac_LastKey = (IPTR)ent;

	    return 0;
	}
	eac->eac_Entries++;
	last = ead;
	ead = ead->ed_Next;
	ent = (struct fnode *)ent->node.mln_Succ;

    } while (ent->node.mln_Succ != NULL);

    last->ed_Next = NULL;
    eac->eac_LastKey = (IPTR)ent;

    return ERROR_NO_MORE_ENTRIES;
}


static LONG rename_object(struct rambase *rambase,
			  struct filehandle *filehandle,
			  CONST_STRPTR namea, CONST_STRPTR nameb)
{
    struct dnode *file = filehandle->node;
    LONG error = 0, i;

    struct dnode *nodea = filehandle->node, *nodeb;
    STRPTR dira, dirb, pos = nameb;

    error = findname(rambase, &pos, &nodea);
    if (!error)
	return ERROR_OBJECT_EXISTS;

    error = findname(rambase, &namea, &file);
    if (error)
	return error;

    if ((struct dnode *)file->volume == file)
        return ERROR_OBJECT_WRONG_TYPE;

    if (file->usecount != 0)
        return ERROR_OBJECT_IN_USE;

    dira = Strdup(rambase, namea);
    dirb = Strdup(rambase, nameb);

    pos = strrchr(dira, '/');
    if (pos)
    {
	*pos = '\0';
	namea = ++pos;
    }
    else
	*dira = '\0';

    pos = strrchr(dirb, '/');
    if (pos)
    {
	*pos = '\0';
	nameb = ++pos;
    }
    else
	*dirb = '\0';

    D(bug("[Ram] rename (%s,%s)=>(%s,%s)\n", dira, namea, dirb, nameb));

    if (*dira == '\0' && *dirb == '\0')
    {
	Strfree(rambase, file->name);
	file->name = Strdup(rambase, nameb);
    }
    else
    {
	nodea = filehandle->node;
	error = findname(rambase, &dira, &nodea);
	if (error)
	    goto cleanup;

	nodeb = filehandle->node;
	error = findname(rambase, &dirb, &nodeb);
	if (error)
	    goto cleanup;

	if (nodeb != nodea)
	{
	    Remove((struct Node *)file);
	    AddTail((struct List *)&nodeb->list, (struct Node *)file);
	}

	Strfree(rambase, file->name);
	file->name = Strdup(rambase, nameb);
    }

    DateStamp(&file->date);

    switch (file->type)
    {
    case ST_FILE:
	Notify_fileChange(rambase, NOTIFY_Delete, (struct fnode *)file);
	Notify_fileChange(rambase, NOTIFY_Add, (struct fnode *)file);
	break;
    case ST_USERDIR:
    default:
	Notify_directoryChange(rambase, NOTIFY_Delete, file);
	Notify_directoryChange(rambase, NOTIFY_Add, file);
	break;
    }

cleanup:
    Strfree(rambase, dirb);
    Strfree(rambase, dira);

    return error;
}

static LONG delete_object(struct rambase *rambase,
			  struct filehandle *filehandle, CONST_STRPTR name)
{
    struct dnode *file = filehandle->node;
    LONG error;

    error = findname(rambase, &name, &file);

    if (error)
    {
	return error;
    }

    if ((struct dnode *)file->volume == file)
    {
	return ERROR_OBJECT_WRONG_TYPE;
    }

    if (file->usecount != 0)
    {
	return ERROR_OBJECT_IN_USE;
    }

    if (file->protect & FIBF_DELETE)
    {
	return ERROR_DELETE_PROTECTED;
    }

    if (file->type == ST_USERDIR && file->list.mlh_Head->mln_Succ != NULL)
    {
	return ERROR_DIRECTORY_NOT_EMPTY;
    }

    delete(rambase, (struct fnode *)file);

    return 0;
}


static int GM_UNIQUENAME(Close)
(
    LIBBASETYPEPTR rambase,
    struct IOFileSys *iofs
)
{
    struct cnode  *dev;
    struct vnode  *vol;
    struct dnode  *dir;
    struct fnode  *file;
    
    struct filehandle *handle;
    
    handle = (struct filehandle *)iofs->IOFS.io_Unit;
    dev = (struct cnode *)handle->node;
    vol = dev->volume;
    
    if (dev->type != ST_LINKDIR || dev->self != dev)
    {
	iofs->io_DosError = ERROR_OBJECT_WRONG_TYPE;

	return 0;
    }

    if (vol->volcount)
    {
	iofs->io_DosError = ERROR_OBJECT_IN_USE;

	return 0;
    }

    free_lock(rambase, handle);
    RemDosEntry(vol->doslist);
    FreeDosEntry(vol->doslist);
    
    while (vol->list.mlh_Head->mln_Succ != NULL)
    {
	dir = (struct dnode *)vol->list.mlh_Head;
	
	if (dir->type == ST_USERDIR)
	{
	    while((file = (struct fnode *)RemHead((struct List *)&dir->list)) != NULL)
	    {
		AddTail((struct List *)&vol->list, (struct Node *)dir);
	    }
	}

	delete(rambase, (struct fnode *)dir);
    }

    Strfree(rambase, vol->name);
    FreeMem(vol, sizeof(struct vnode));

    Strfree(rambase, dev->name);
    FreeMem(dev, sizeof(struct cnode));

    iofs->io_DosError = 0;

    return TRUE;
}


void processFSM(struct rambase *rambase);


static void deventry(struct rambase *rambase)
{
    ULONG notifyMask;
    ULONG fileOpMask;
    struct Message *msg;

    /*
	Init device port. AllocSignal() cannot fail because this is a
	freshly created task with all signal bits still free.
    */

    rambase->port->mp_SigBit = AllocSignal(-1);
    rambase->port->mp_Flags  = PA_SIGNAL;

    /* Check if there are pending messages that we missed */
    if ((msg = GetMsg(rambase->port))) PutMsg(rambase->port, msg);

    Notify_initialize(rambase);

    notifyMask = 1 << rambase->notifyPort->mp_SigBit;

    fileOpMask = 1 << rambase->port->mp_SigBit;

    while (TRUE)
    {
	ULONG flags;

	flags = Wait(fileOpMask | notifyMask);

	if (flags & notifyMask)
	{
	    struct NotifyMessage *nMessage;

	    D(kprintf("Got replied notify message\n"));

	    while ((nMessage = (struct NotifyMessage *)GetMsg(rambase->notifyPort)) != NULL)
	    {
		nMessage->nm_NReq->nr_Flags &= ~NRF_NOT_REPLIED;
		FreeVec(nMessage);
	    }
	}

	if (flags & fileOpMask)
	{
	    processFSM(rambase);
	}
    }
}


void processFSM(struct rambase *rambase)
{
    struct IOFileSys  *iofs;
    struct dnode      *dir;

    LONG   error = 0;

    /* Get and process the messages. */
    while ((iofs = (struct IOFileSys *)GetMsg(rambase->port)) != NULL)
    {
	D(bug("Ram.handler initialized %u\n", iofs->IOFS.io_Command));

	switch (iofs->IOFS.io_Command)
	{
	case FSA_OPEN:
	    /*
	      get handle on a file or directory
	      Unit *current; current directory / new handle on return
	      STRPTR name;   file- or directoryname
	      LONG mode;     open mode
	    */
	    
	    error = open_(rambase,
			  (struct filehandle **)&iofs->IOFS.io_Unit,
			  iofs->io_Union.io_OPEN.io_Filename,
			  iofs->io_Union.io_OPEN.io_FileMode);
	    break;
	    
	case FSA_OPEN_FILE:
	    /*
	      open a file or create a new one
	      Unit *current; current directory / new handle on return
	      STRPTR name;   file- or directoryname
	      LONG mode;     open mode
	      LONG protect;  protection flags if a new file is created
	    */
	    
	    error = open_file(rambase,
			      (struct filehandle **)&iofs->IOFS.io_Unit,
			      iofs->io_Union.io_OPEN_FILE.io_Filename,
			      iofs->io_Union.io_OPEN_FILE.io_FileMode,
			      iofs->io_Union.io_OPEN_FILE.io_Protection);
	    break;
	    
	case FSA_READ:
	    /*
	      read a number of bytes from a file
	      Unit *current; filehandle
	      APTR buffer;   data
	      LONG numbytes; number of bytes to read /
	      number of bytes read on return,
	      0 if there are no more bytes in the file
	    */
	    error = read(rambase,
			 (struct filehandle *)iofs->IOFS.io_Unit,
			 iofs->io_Union.io_READ.io_Buffer,
			 &iofs->io_Union.io_READ.io_Length);
	    break;

	case FSA_WRITE:
	    /*
	      write a number of bytes to a file
	      Unit *current; filehandle
	      APTR buffer;   data
	      LONG numbytes; number of bytes to write /
	      number of bytes written on return
	    */
	    error = write(rambase,
			      (struct filehandle *)iofs->IOFS.io_Unit,
			  iofs->io_Union.io_WRITE.io_Buffer,
			  &iofs->io_Union.io_WRITE.io_Length);
	    break;
		
	case FSA_SEEK:
	    /*
	      set / read position in file
	      Unit *current; filehandle
	      LONG posh;
	      LONG posl;     relative position /
	      old position on return
	      LONG mode;     one of OFFSET_BEGINNING, OFFSET_CURRENT,
	      OFFSET_END
	    */
	    error = seek(rambase,
			 (struct filehandle *)iofs->IOFS.io_Unit,
			 &iofs->io_Union.io_SEEK.io_Offset,
			 iofs->io_Union.io_SEEK.io_SeekMode);
	    break;
	    
	case FSA_CLOSE:
	    {
		/* Get rid of a handle
		   Unit *current; filehandle */
		
		struct filehandle *handle = 
		    (struct filehandle *)iofs->IOFS.io_Unit;
		
		Notify_fileChange(rambase, NOTIFY_Close,
				  (struct fnode *)handle->node);
		error = free_lock(rambase, handle);
		break;
	    }
	    
	case FSA_EXAMINE:
	    /*
	      Get information about the current object
	      Unit *current; current object
	      struct ExAllData *ead; buffer to be filled
	      ULONG size;    size of the buffer
	      ULONG type;    type of information to get
	      iofs->io_DirPos; leave current position so
	      ExNext() knows where to find
	      next object
	    */
	    error = examine((struct fnode *)((struct filehandle *)iofs->IOFS.io_Unit)->node,
			    iofs->io_Union.io_EXAMINE.io_ead,
			    iofs->io_Union.io_EXAMINE.io_Size,
			    iofs->io_Union.io_EXAMINE.io_Mode,
			    &(iofs->io_DirPos));
	    break;
	    
	case FSA_EXAMINE_NEXT:
	    /*
	      Get information about the next object 
	      Unit *current; current object
	      struct FileInfoBlock *fib; 
	    */
	    error = examine_next(rambase,
				 (struct filehandle *)iofs->IOFS.io_Unit,
				 iofs->io_Union.io_EXAMINE_NEXT.io_fib);
	    break;
	    
	case FSA_EXAMINE_ALL:
	    /*
	      Read the current directory
	      Unit *current; current directory
	      struct ExAllData *ead; buffer to be filled
	      ULONG size;    size of the buffer
	      ULONG type;    type of information to get
	    */
	    error = examine_all((struct filehandle *)iofs->IOFS.io_Unit,
				iofs->io_Union.io_EXAMINE_ALL.io_ead,
				iofs->io_Union.io_EXAMINE_ALL.io_eac,
				iofs->io_Union.io_EXAMINE_ALL.io_Size,
				iofs->io_Union.io_EXAMINE_ALL.io_Mode);
	    break;
	    
	case FSA_CREATE_DIR:
	    /*
	      Build lock and open a new directory
	      Unit *current; current directory
	      STRPTR name;   name of the dir to create
	      LONG protect;  Protection flags for the new dir
	    */
	    
	    // kprintf("Creating directory %s\n",
	    //	iofs->io_Union.io_CREATE_DIR.io_Filename);
	    
	    
	    error = create_dir(rambase,
			       (struct filehandle **)&iofs->IOFS.io_Unit,
			       iofs->io_Union.io_CREATE_DIR.io_Filename,
			       iofs->io_Union.io_CREATE_DIR.io_Protection);
	    break;
	    
	case FSA_DELETE_OBJECT:
	    /*
	      Delete file or directory
	      Unit *current; current directory
	      STRPTR name;   filename
	    */
	    error = delete_object(rambase,
				  (struct filehandle *)iofs->IOFS.io_Unit,
				  iofs->io_Union.io_DELETE_OBJECT.io_Filename);
	    break;
	    
	case FSA_SET_PROTECT:
	    {
		/*
		  Set protection bits for a certain file or directory.
		  Unit *current; current directory
		  STRPTR name;   filename
		  ULONG protect; new protection bits
		*/
		
		CONST_STRPTR name = iofs->io_Union.io_SET_PROTECT.io_Filename;
		
		dir = ((struct filehandle *)iofs->IOFS.io_Unit)->node;
		error = findname(rambase, &name, &dir);
		
		if (!error)
		{
		    dir->protect = iofs->io_Union.io_SET_PROTECT.io_Protection;
		}
		
		break;
	    }		
	    
	case FSA_SET_OWNER:
	    {
		/*
		  Set owner and group of the file or directory
		  Unit *current; current directory
		  STRPTR name;   filename
		  ULONG UID;
		  ULONG GID;
		*/
		
		CONST_STRPTR name = iofs->io_Union.io_SET_OWNER.io_Filename;
		
		dir = ((struct filehandle *)iofs->IOFS.io_Unit)->node;
		error = findname(rambase, &name, &dir);
		
		if(!error)
		{
		}
		
		break;
	    }
	    
	case FSA_SET_DATE:
	    {
		/*
		  Set creation date of the file
		  Unit *current; current directory
		  STRPTR name;   filename
		  ULONG days;
		  ULONG mins;
		  ULONG ticks;   timestamp
		*/
		
		CONST_STRPTR name = iofs->io_Union.io_SET_DATE.io_Filename;
		
		dir = ((struct filehandle *)iofs->IOFS.io_Unit)->node;
		error = findname(rambase, &name, &dir);
		
		if(!error)
		{
		    dir->date = iofs->io_Union.io_SET_DATE.io_Date;
		}
		
		break;
	    }
	    
	case FSA_SET_COMMENT:
	    {
		/*
		  Set a comment for the file or directory;
		  Unit *current; current directory
		  STRPTR name;   filename
		  STRPTR comment; NUL terminated C string or NULL.
		*/

		CONST_STRPTR name = iofs->io_Union.io_SET_COMMENT.io_Filename;
		
		dir = ((struct filehandle *)iofs->IOFS.io_Unit)->node;
		error = findname(rambase, &name, &dir);
		
		if (!error)
		{
		    if (iofs->io_Union.io_SET_COMMENT.io_Comment)
		    {
			STRPTR s = Strdup(rambase,
					  iofs->io_Union.io_SET_COMMENT.io_Comment);
			if (s != NULL)
			{
			    Strfree(rambase, dir->comment);
			    dir->comment = s;
			}
			else
			{
			    error = ERROR_NO_FREE_STORE;
			}
		    }
		    else
		    {
			Strfree(rambase, dir->comment);
			dir->comment = NULL;
		    }
		}
		
		break;
	    }
	    
	case FSA_SET_FILE_SIZE:
	    /*
	      Set a new size for the file.
	      Unit *file;    filehandle
	      LONG offh;
	      LONG offl;     offset to current position/
	      new size on return
	      LONG mode;     relative to what (see Seek)
	    */
	    error = set_file_size(rambase,
				  (struct filehandle *)iofs->IOFS.io_Unit,
				  &iofs->io_Union.io_SET_FILE_SIZE.io_Offset,
				  iofs->io_Union.io_SET_FILE_SIZE.io_SeekMode);
	    break;
	    
	case FSA_IS_FILESYSTEM:
	    iofs->io_Union.io_IS_FILESYSTEM.io_IsFilesystem = TRUE;
	    error = 0;
	    break;
		
	case FSA_DISK_INFO:
	    {
		struct InfoData *id = iofs->io_Union.io_INFO.io_Info;
		
		id->id_NumSoftErrors = 0;
		id->id_UnitNumber = 0;
		id->id_DiskState = ID_VALIDATED;
		id->id_NumBlocks = AvailMem(MEMF_TOTAL | MEMF_PUBLIC)/BLOCKSIZE;
		id->id_NumBlocksUsed = id->id_NumBlocks - AvailMem(MEMF_PUBLIC)/BLOCKSIZE;
		id->id_BytesPerBlock = BLOCKSIZE;
		id->id_DiskType = ID_DOS_DISK;
		id->id_VolumeNode = NULL;        /* What is this? */
		id->id_InUse = (LONG)TRUE;
		
		error = 0;
	    }
	    
	    break;
	    
	    
	case FSA_ADD_NOTIFY:
	    {
		BOOL ok;
		struct NotifyRequest *nr = 
		    iofs->io_Union.io_NOTIFY.io_NotificationRequest;
		struct filehandle *fh = (struct filehandle *)iofs->IOFS.io_Unit;
		
		D(kprintf("Adding notification for entity (nr = %s)\n",
			  nr->nr_FullName));
		
		ok = Notify_addNotification(rambase, fh->node, nr);

		if (!ok)
		{
		    error = ERROR_NO_FREE_STORE;
		}
		
		D(kprintf("Notification now added!\n"));
	    }
	    break;
	    
	case FSA_REMOVE_NOTIFY:
	    {
		struct NotifyRequest *nr = 
		    iofs->io_Union.io_NOTIFY.io_NotificationRequest;
		
		Notify_removeNotification(rambase, nr);
	    }
	    break;

	case FSA_RENAME:
	    error = rename_object(rambase,
				  (struct filehandle *)iofs->IOFS.io_Unit,
				  iofs->io_Union.io_RENAME.io_Filename,
				  iofs->io_Union.io_RENAME.io_NewName);
	    break;

	case FSA_IS_INTERACTIVE:
	    iofs->io_Union.io_IS_INTERACTIVE.io_IsInteractive = 0;
            break;
	    
	default:
	    error = ERROR_NOT_IMPLEMENTED;
	    
	    D(kprintf("ram_handler, unimplemented FSA: %ld\n", iofs->IOFS.io_Command));
	    break;
 /*
   FSA_FILE_MODE
   Change or read the mode of a single filehandle
   Unit *current; filehandle to change
   ULONG newmode; new mode/old mode on return
   ULONG mask;    bits affected
   
   FSA_MOUNT_MODE
   Change or read the mode of the filesystem
   Unit *current; filesystem to change
   ULONG newmode; new mode/old mode on return
   ULONG mask;    bits affected
   STRPTR passwd; password for MMF_LOCKED
   
   FSA_MAKE_HARDLINK
   Create a hard link on a file, directory or soft link
   Unit *current; current directory
   STRPTR name;   softlink name
   Unit *target;  target handle
   
   FSA_MAKE_SOFTLINK
   Create a soft link to another object
   Unit *current; current directory
   STRPTR name;   softlink name
   STRPTR target; target name
   
   FSA_READ_LINK
   FSA_SERIALIZE_DISK
   FSA_WAIT_CHAR
   FSA_INFO
   FSA_TIMER
   FSA_DISK_TYPE
   FSA_DISK_CHANGE
   FSA_SAME_LOCK
   FSA_CHANGE_SIGNAL
   FSA_FORMAT
   FSA_EXAMINE_ALL
   FSA_EXAMINE_FH
   FSA_EXAMINE_ALL_END
   
 */
	}
	
	iofs->io_DosError = error;
	ReplyMsg(&iofs->IOFS.io_Message);
    }

#if 0
    if (rambase->iofs != NULL)
    {
	iofs = rambase->iofs;
	
	if (iofs->IOFS.io_Message.mn_Node.ln_Type == NT_MESSAGE)
	{
	    abort_notify(rambase, iofs);
	    iofs->io_DosError = ERROR_BREAK;
	    rambase->iofs = NULL;
	    ReplyMsg(&iofs->IOFS.io_Message);
	}
	else
	{
	    rambase->iofs = NULL;
	    Signal(1, 0);
	}
    }
#endif
}





/***************************************************************************
****************************************************************************/

/**  Notification stuff  **/

/*
 * Immediate notification:
 *
 * ACTION_RENAME_OBJECT
 * ACTION_RENAME_DISK
 * ACTION_CREATE_DIR
 * ACTION_DELETE_OBJECT
 * ACTION_SET_DATE
 *
 *
 * Session semantics notification:
 *
 * ACTION_WRITE
 * ACTION_FINDUPDATE
 * ACTION_FINDOUTPUT
 * ACTION_SET_FILE_SIZE
 *
 *
 * For ram-handler, the only FSA actions currently implemented/affected are
 * FSA_WRITE, FSA_CREATE_DIR, FSA_DELETE (and implicitly FSA_OPEN, FSA_CLOSE)
 *
 */


BOOL Notify_initialize(struct rambase *rambase)
{
    rambase->notifyPort = CreateMsgPort();
    
    if (rambase->notifyPort == NULL)
    {
	return FALSE;
    }

    return TRUE;
}


void Notify_fileChange(struct rambase *rambase, NType type, struct fnode *file)
{
    switch (type)
    {
    case NOTIFY_Open:
	break;

    case NOTIFY_Write:
	D(bug("Setting write flag!\n"));
	file->flags |= FILEFLAGS_Changed;
	break;

    case NOTIFY_Close:
	/* Only notify in case the file was changed */
	if (file->flags & FILEFLAGS_Changed)
	{
	    D(bug("Notification on close (file was changed).\n"));
	    file->flags &= ~FILEFLAGS_Changed;
	    Notify_notifyTasks(rambase, &file->receivers);
	}
	break;

    case NOTIFY_Add:
	{
	    STRPTR fullName;
	    struct List *receivers;
	    
	    fullName = getName(rambase, (struct dnode *)file);

	    D(kprintf("Found full name: %s\n", fullName));

	    receivers = HashTable_find(rambase, rambase->notifications,
				       fullName);      

	    if (receivers != NULL)
	    {
		D(kprintf("Found notification for created file!\n"));

		while (!IsListEmpty(receivers))
		{
		    AddHead((struct List *)&file->receivers,
			    RemHead(receivers));
		}
	    }

	    HashTable_remove(rambase, rambase->notifications, fullName);

	    D(bug("Notifying file receivers!\n"));
	    Notify_notifyTasks(rambase, &file->receivers);

	    FreeVec(fullName);
	}
    	break;
	
    case NOTIFY_Delete:
	/* It's the same thing as for directories... */
	Notify_directoryChange(rambase, NOTIFY_Delete, (struct dnode *)file);
	break;

    default:
	kprintf("Error\n");
	break;
    }
}


void Notify_directoryChange(struct rambase *rambase, NType type,
			    struct dnode *dir)
{
    switch (type)
    {
    case NOTIFY_Open:
	{
	    STRPTR fullName;

	    fullName = getName(rambase, dir);

	    /* TODO:    HashTable_apply(ht, fullName, dir); */

	    FreeVec(fullName);
	}
	break;

    case NOTIFY_Add:
	{
	    STRPTR fullName;
	    struct List *receivers;

	    fullName = getName(rambase, dir);

	    D(kprintf("Found full name: %s\n", fullName));

	    receivers = HashTable_find(rambase, rambase->notifications,
				       fullName);      

	    if (receivers != NULL)
	    {
		D(kprintf("Found notification for created directory!\n"));

		while (!IsListEmpty(receivers))
		{
		    AddHead((struct List *)&dir->receivers,
			    RemHead(receivers));
		}
	    }

	    HashTable_remove(rambase, rambase->notifications, fullName);

	    FreeVec(fullName);
	}

	D(kprintf("Notifying dir receivers!\n"));
	Notify_notifyTasks(rambase, &dir->receivers);
	break;

    case NOTIFY_Delete:
	{
	    /* As we have deleted a file, we must move the requests for
	       this file to the hash table */
	    struct List *receivers = (struct List *)&dir->receivers;

	    Notify_notifyTasks(rambase, &dir->receivers);

	    /* Move the Notification request to the hash table as we're going
	       to delete this directory */
	    while (!IsListEmpty(receivers))
	    {
		struct Receiver *rr = (struct Receiver *)RemHead(receivers);

		HashTable_insert(rambase, rambase->notifications,
				 rr->nr->nr_FullName, rr);
	    }
	}

	break;

    default:
	kprintf("Error\n");
	break;
    }

    // kprintf("Returning from Notify_dirChange()\n");
}


/* TODO: Implement support for NRF_WAIT_REPLY */
void Notify_notifyTasks(struct rambase *rambase, struct MinList *notifications)
{
    struct Receiver *rr;

    // kprintf("Inside notifytasks, not = %p\n", notifications);

    ForeachNode((struct List *)notifications, rr)
    {
	struct NotifyRequest *nr = rr->nr;

	if (nr->nr_Flags & NRF_SEND_MESSAGE &&
	    !(nr->nr_Flags & NRF_WAIT_REPLY && nr->nr_Flags & NRF_NOT_REPLIED))
	{
	    struct NotifyMessage *nm = AllocVec(sizeof(struct NotifyMessage),
						MEMF_PUBLIC | MEMF_CLEAR);

	    if (nm == NULL)
	    {
		return;
	    }

	    nm->nm_ExecMessage.mn_ReplyPort = rambase->notifyPort;
	    nm->nm_ExecMessage.mn_Length = sizeof(struct NotifyMessage);
	    nm->nm_NReq = nr;
	    nr->nr_Flags |= NRF_NOT_REPLIED;

	    PutMsg(nr->nr_stuff.nr_Msg.nr_Port, &nm->nm_ExecMessage);
	}
	else if (nr->nr_Flags & NRF_SEND_SIGNAL)
	{
	    Signal(nr->nr_stuff.nr_Signal.nr_Task,
		   1 << nr->nr_stuff.nr_Signal.nr_SignalNum);
	}
    }
}

/* also defined in rom/dos/filesystem_support.c */
STRPTR RamStripVolume(STRPTR name)
{
    STRPTR path = strchr(name, ':');
    if (path != NULL)
        path++; 
    else
        path = name;
    return path;
}

BOOL Notify_addNotification(struct rambase *rambase, struct dnode *dn, 
			    struct NotifyRequest *nr)
{
    struct dnode *dnTemp = dn;
    HashTable *ht = rambase->notifications;
    STRPTR name = nr->nr_FullName;
    CONST_STRPTR  tname = RamStripVolume(name);

    /* First: Check if the file is opened */
    D(bug("Checking existence of %s\n", tname));

    if (findname(rambase, &tname, &dnTemp) == 0)
    {
	/* This file was already opened (or at least known by the file
	   system) */

	struct Receiver *rr = AllocVec(sizeof(struct Receiver), MEMF_CLEAR);
	
	if (rr == NULL)
	{
	    return FALSE;
	}
	
	rr->nr = nr;
		
	if (nr->nr_Flags & NRF_NOTIFY_INITIAL)
	{
	    struct MinList tempList;
	    
	    /* Create a temporary list on the stack and add the receiver to
	       it. Then forget about this and add the node to the receiver
	       list (below this block). */
	    NewList((struct List *)&tempList);
	    AddHead((struct List *)&tempList, &rr->node);
	    Notify_notifyTasks(rambase, &tempList);
	}

	/* Add the receiver node to the file's/directory's list of receivers */
	AddTail((struct List *)&dnTemp->receivers, &rr->node);

	D(bug("Notification added to node: %s\n", name));

	return TRUE;
    }
    else
    {
	/* This file is not opened */

	struct Receiver *rr = AllocVec(sizeof(struct Receiver), MEMF_CLEAR);

	if (rr == NULL)
	{
	    return FALSE;
	}
	
	rr->nr = nr;

	HashTable_insert(rambase, ht, name, rr);

	return TRUE;
    }
}


void Notify_removeNotification(struct rambase *rambase,
			       struct NotifyRequest *nr)
{
    struct dnode *dn = (struct dnode *)rambase->root;
    struct Receiver *rr, *rrTemp;
    STRPTR name = nr->nr_FullName;
    CONST_STRPTR  tname = RamStripVolume(name);
    struct List *receivers;
    BOOL  fromTable = FALSE;

    if (findname(rambase, &tname, &dn) == 0)
    {
	receivers = (struct List *)&dn->receivers;
    }
    else
    {
	/* This was a request for an entity that doesn't exist yet */
	receivers = HashTable_find(rambase, rambase->notifications, name);

	if (receivers == NULL)
	{
	    D(bug("Ram: This should not happen! -- buggy application...\n"));
	    return;
	}

	fromTable = TRUE;
    }

    ForeachNodeSafe(&receivers, rr, rrTemp)
    {
	if (rr->nr == nr)
	{
	    Remove((struct Node *)rr);
	    FreeVec(rr);
	    break;
	}
    }

    if (fromTable)
    {
	/* If we removed a request for a file that doesn't exist yet --
	   if this was the only notification request for that file, remove
	   also the hash entry. */
	if (IsListEmpty((struct List *)receivers))
	{
	    HashTable_remove(rambase, rambase->notifications, name);
	}
    }
}

static STRPTR getName(struct rambase *rambase, struct dnode *dn)
{
    struct dnode *dnTemp = dn;

    ULONG   length;
    STRPTR  fullName;
    CONST_STRPTR  slash = "/";
    CONST_STRPTR  nextSlash = slash;
    ULONG   partLen;

    /* First, calculate the size of the complete filename */
    length = strlen(dn->name) + 2; /* Add trailing 0 byte and possible '/' */

    while (findname(rambase, &slash, &dnTemp) == 0)
    {
	slash = nextSlash;
	length += 1 + strlen(dnTemp->name);	    /* Add '/' byte */
    }

    /* The MEMF_CLEAR is necessary for the while loop below to work */
    fullName = AllocVec(length, MEMF_PUBLIC | MEMF_CLEAR);

    if (fullName == NULL)
    {
	return NULL;
    }

    dnTemp = dn;
    fullName += length - 1;

    fullName -= strlen(dn->name) + 1;
    strcpy(fullName, dn->name);

    slash = "/";
    nextSlash = slash;
    while (findname(rambase, &slash, &dnTemp) != ERROR_OBJECT_NOT_FOUND)
    {
	slash = nextSlash;
	partLen = strlen(dnTemp->name);
	    
	fullName -= partLen + 1;
	strcpy(fullName, dnTemp->name);

	if ((struct vnode *)dnTemp == dnTemp->volume)
	     fullName[partLen] = ':';      /* Root of Ram Disk */
	else
	     fullName[partLen] = '/';      /* Replace 0 with '/' */
    }

    return fullName;
}

ADD2INITLIB(GM_UNIQUENAME(Init),0)
ADD2OPENDEV(GM_UNIQUENAME(Open),0)
ADD2CLOSEDEV(GM_UNIQUENAME(Close),0)
ADD2EXPUNGELIB(GM_UNIQUENAME(Expunge),0)
