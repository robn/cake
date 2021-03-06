/*
    Copyright � 1995-2009, The AROS Development Team. All rights reserved.
    $Id$

    POSIX function opendir().
*/

#include <dos/dos.h>
#include <proto/dos.h>

#include <stdlib.h>
#ifndef ExNext_IS_WORKING_WITHOUT_ASSIGN
#include <stdio.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "__open.h"
#include "__errno.h"
#include "__upath.h"

#define DEBUG 0
#include <aros/debug.h>

/*****************************************************************************

    NAME */
#include <dirent.h>

	DIR *opendir(

/*  SYNOPSIS */
	const char *name)

/*  FUNCTION
	Opens a directory

    INPUTS
	pathname - Path and filename of the directory you want to open.

    RESULT
	NULL for error or a directory stream

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
 	open(), readdir(), closedir(), rewinddir(), seekdir(),
	telldir(), scandir()

    INTERNALS

******************************************************************************/
{
    DIR *dir;
    int fd;
    fcb *cblock;
    fdesc *desc;
    BPTR lock;
    char *aname;
#ifndef ExNext_IS_WORKING_WITHOUT_ASSIGN
    char assign[32];
#endif

    if (!name)
    {
    	errno = EFAULT;
	goto err1;
    }

    dir = malloc(sizeof(DIR));
    if (!dir)
    {
	errno = ENOMEM;
	goto err1;
    }

    dir->priv = AllocDosObject(DOS_FIB, NULL);
    if (!dir->priv)
    {
	errno = ENOMEM;
	goto err2;
    }

    /* Lock is used instead of open to allow opening "" */
    aname = __path_u2a(name);
    lock = Lock(aname, SHARED_LOCK);
    if (!lock)
    {
	errno = IoErr2errno(IoErr());
	goto err3;
    }

#ifndef ExNext_IS_WORKING_WITHOUT_ASSIGN
    sprintf(assign, "READDIR%x", (unsigned)dir);

    if (!AssignLock(assign, DupLock(lock)))
    {
	D(bug("!AssignLock err=%d\n", IoErr()));
    }

    UnLock(lock);
    lock = Lock(aname, SHARED_LOCK);

    AssignLock(assign, NULL);
#endif

    if (!Examine(lock, dir->priv))
    {
	errno = IoErr2errno(IoErr());
	goto err4;
    }

    if (((struct FileInfoBlock *)dir->priv)->fib_DirEntryType<=0)
    {
	errno = ENOTDIR;
	goto err4;
    }

    cblock = AllocVec(sizeof(fcb), MEMF_ANY | MEMF_CLEAR);
    if(!cblock)
    {
        errno = ENOMEM;
        goto err4;
    }
    desc = __alloc_fdesc();
    if(!desc)
    {
        errno = ENOMEM;
        goto err5;
    }
    desc->fdflags = 0;
    desc->fcb = cblock;
    desc->fcb->fh = lock;
    desc->fcb->flags = O_RDONLY;
    desc->fcb->opencount = 1;
    desc->fcb->isdir = 1;

    fd = __getfdslot(__getfirstfd(3));
    __setfdesc(fd, desc);

    dir->fd = fd;
    dir->pos = 0;
    dir->ent.d_name[NAME_MAX] = '\0';

    D(bug("opendir(%s) fd=%d\n", name, fd));
    return dir;

err5:
    FreeVec(cblock);
err4:
    UnLock(lock);
err3:
    FreeDosObject(DOS_FIB, dir->priv);
err2:
    free(dir);
err1:
    D(bug("opendir(%s) errno=%d\n", name, errno));
    return NULL;
}
