#ifndef DOS_FILESYSTEM_H
#define DOS_FILESYSTEM_H

/*
    (C) 1995-97 AROS - The Amiga Research OS
    $Id$

    Desc: AROS specific structures and definitions for filesystems.
    Lang: english
*/
#ifndef EXEC_IO_H
#   include <exec/io.h>
#endif
#ifndef DOS_DOS_H
#   include <dos/dos.h>
#endif
#ifndef DOS_FILEHANDLER_H
#   include <dos/filehandler.h>
#endif
#ifndef DOS_EXALL_H
#   include <dos/exall.h>
#endif

/* This file is AROS specific. Do not use the structures and #defines contained
   in here if you want to stay compatible with AmigaOS! */


/* Filesystem handlers are called with so-called actions, whenever they are
   supposed to do something. See below for a list of currently defined actions
   and how to use them.

   Not all filesystems have to support all actions. Filesystems have to return
   ERROR_ACTION_NOT_KNOWN (<dos/dos.h>) in IOFileSys->io_DosError, if they do
   not support the specified action. If they know an action but ignore it on
   purpose, they may return ERROR_NOT_IMPLEMENTED. A purpose may be that the
   hardware or software the filesystem relies on does not support this kind of
   action (eg: a net-filesystem protocol may not support renaming of files, so
   a filesystem handler for this protocol should return ERROR_NOT_IMPLEMENTED
   for FSA_RENAME). Another example is the nil-device, which does not implement
   FSA_CREATE_DIR or anything concerning specific files. What does that mean
   for an application? If an application receives ERROR_NOT_IMPLEMENTED, it
   knows that the action, it wanted to perform, makes no sense for that
   filesystem. If it receives ERROR_ACTION_NOT_KNOWN, it knows that the
   filesystem does not know about this action for whatever reason (including
   that it makes no sense to perform that action on that specific filesystem).

   All actions work relative to the current directory (where it makes sense),
   set in the IOFileSys->IOFS.io_Unit field. This field also serves as a
   pointer to filehandles for actions that either need a filehandle as argument
   or return one. When not explicitly stated otherwise this field has to be set
   to the filehandle to affect or is set to the filehandle that is returned by
   the action. Note that the filehandle mentioned above is not a pointer to a
   struct FileHandle as defined in <dos/dosextens.h>, but is an APTR to a
   device specific blackbox structure. In a struct FileHandle this private
   pointer is normally to be found in FileHandle->fh_Unit.

   Whenever a filename is required as argument, this filename has to be
   stripped from the devicename, ie it has to be relative to the current
   directory on that volume (set in the io_Unit field). */

/* Returns a new filehandle. The file may be newly created (depending on
   io_FileMode. */
#define FSA_OPEN 1
struct IFS_OPEN
{
    STRPTR io_Filename; /* File to open. */
    ULONG  io_FileMode; /* see below */
};

/* Closes an opened filehandle. Takes no extra arguments. */
#define FSA_CLOSE 2

/* Reads from a filehandle into a buffer. */
#define FSA_READ 3
struct IFS_READ_WRITE
{
      /* The buffer for the data to read/to write. */
    char * io_Buffer;
      /* The length of the buffer. This is filled by the filesystem handler
         with the number of bytes actually read/written. */
    LONG   io_Length;
};

/* Writes the contents of a buffer into a filehandle. Uses IFS_READ_WRITE. */
#define FSA_WRITE 4

/* The action does exactly the same as the function Seek(). */
#define FSA_SEEK 5
struct IFS_SEEK
{
      /* Offset from position, specified as mode. This is filled by the
         filehandler with the old position in the file. */
      QUAD io_Offset; 
      /* Seek mode as defined in <dos/dos.h> (OFFSET_#?). */
    LONG io_SeekMode;
};

/* Sets the size of filehandle. Uses IFS_SEEK (see above) as argument array. */
#define FSA_SET_FILE_SIZE 6

/* Waits for a character to arrive at the filehandle. This is not used for
   plain files, but for queues only. Optionally a maximum time to wait may be
   specified. */
#define FSA_WAIT_CHAR 7
struct IFS_WAIT_CHAR
{
      /* Maximum time (in microseconds) to wait for a character. */
    LONG io_Timeout;
      /* This is set to FALSE by the filehandler, if no character arrived in
         time. Otherwise it is set to TRUE. */
    BOOL io_Success;
};

/* Applies a new mode to a file. If you supply io_Mask with a value of 0,
   no changes are made and you can just read the resulting io_FileMode. */
#define FSA_FILE_MODE 8
struct IFS_FILE_MODE
{
      /* The new mode to apply to the filehandle. See below for definitions.
         The filehandler fills this with the old mode bits. */
    ULONG io_FileMode;
      /* This mask defines, which flags are to be changed. */
    ULONG io_Mask;
};

/* This action can be used to query if a filehandle is interactive, ie if it
   is a terminal or not. */
#define FSA_IS_INTERACTIVE 9
struct IFS_IS_INTERACTIVE
{
      /* This boolean is filled by the filehandler. It is set to TRUE, if the
         filehandle is interactive, otherwise it is set to FALSE. */
    BOOL io_IsInteractive;
};

/* Compares two locks for equality. */
#define FSA_SAME_LOCK 10
struct IFS_SAME_LOCK
{
    APTR io_Lock[2]; /* The two locks to compare. */
    LONG io_Same;    /* This set to one of LOCK_DIFFERENT or LOCK_SAME (see
                        <dos/dos.h>). */
};

/* Examines a filehandle, giving various information about it. */
#define FSA_EXAMINE 11
struct IFS_EXAMINE
{
      /* ExAllData structure buffer to be filled by the filehandler. */
    struct ExAllData * io_ead;
    LONG               io_Size; /* Size of the buffer. */
      /* With which kind of information shall the buffer be filled with? See
         ED_* definitions in <dos/exall.h> for more information. */
    LONG               io_Mode;
};

#define FSA_EXAMINE_NEXT 12
struct IFS_EXAMINE_NEXT
{
    /* FileInfoBlock structure buffer to be used and filled by the 
       filehandler. */
    struct FileInfoBlock * io_fib;
};

/* Works exactly like FSA_EXAMINE with the exeption that multiple files may be
   examined, ie the filehandle must be a directory. */
#define FSA_EXAMINE_ALL 13

/* This has to be called, if FSA_EXAMINE_ALL is stopped before all examined
   files were returned. It takes no arguments except the filehandle in
   io_Unit. */
#define FSA_EXAMINE_ALL_END 14

/* Works exactly like FSA_OPEN, but you can additionally specify protection
   bits to be applied to new files. */
#define FSA_OPEN_FILE 15
struct IFS_OPEN_FILE
{
    STRPTR io_Filename;   /* File to open. */
    ULONG  io_FileMode;   /* see below */
    ULONG  io_Protection; /* The protection bits. */
};

/* Creates a new directory. The filehandle of that new directory is returned.
*/
#define FSA_CREATE_DIR 16
struct IFS_CREATE_DIR
{
    STRPTR io_Filename;   /* Name of directory to create. */
    ULONG  io_Protection; /* The protection bits. */
};

/* Creates a hard link (ie gives one file a second name). */
#define FSA_CREATE_HARDLINK 17
struct IFS_CREATE_HARDLINK
{
    STRPTR   io_Filename; /* The filename of the link to create. */
    APTR     io_OldFile;  /* Filehandle of the file to link to. */
};

/* Creates a soft link (ie a file is created, which references another by its
   name). */
#define FSA_CREATE_SOFTLINK 18
struct IFS_CREATE_SOFTLINK
{
    STRPTR io_Filename;  /* The filename of the link to create. */
    STRPTR io_Reference; /* The name of the file to link to. */
};

/* Renames a file. To the old and the new name, the current directory is
   applied to. */
#define FSA_RENAME 19
struct IFS_RENAME
{
    STRPTR io_Filename; /* The old filename. */
    STRPTR io_NewName;  /* The new filename. */
};

/* Resolves the full path name of the file a softlink filehandle points to. */
#define FSA_READ_SOFTLINK 20
struct IFS_READ_SOFTLINK
{
      /* The buffer to fill with the pathname. If this buffer is too small, the
         filesystem handler is supposed to return ERROR_LINE_TOO_LONG. */
    STRPTR io_Buffer;
      /* The size of the buffer pointed to by io_Buffer. */
    ULONG  io_Size;
};

/* Deletes an object on the volume. */
#define FSA_DELETE_OBJECT 21
struct IFS_DELETE_OBJECT
{
    STRPTR io_Filename; /* The name of the file to delete. */
};

/* Sets a filecomment for a file. */
#define FSA_SET_COMMENT 22
struct IFS_SET_COMMENT
{
    STRPTR io_Filename; /* The file of the file to be commented. */
    STRPTR io_Comment;  /* The new filecomment. May be NULL, in which case the
                           current filecomment is deleted. */
};

/* Sets the protection bits of a file. */
#define FSA_SET_PROTECT 23
struct IFS_SET_PROTECT
{
    STRPTR io_Filename;   /* The file to change. */
    ULONG  io_Protection; /* The new protection bits. */
};

/* Sets the ownership of a file. */
#define FSA_SET_OWNER 24
struct IFS_SET_OWNER
{
    STRPTR io_Filename; /* The file to change. */
    UWORD  io_UID;      /* The new owner. */
    UWORD  io_GID;      /* The new group owner. */
};

/* Sets the last modification date of the filename given as first argument.
   The date is given as standard TimeStamp structure (see <dos/dos.h>) as
   second to fourth argument (ie as days, minutes and ticks). */
#define FSA_SET_DATE 25
struct IFS_SET_DATE
{
    STRPTR           io_Filename; /* The file to change. */
    struct DateStamp io_Date;     /* The new date. (see <dos/dosextens.h>) */
};

/* Check if a filesystem is in fact a FILEsystem, ie can contain different
   files. */
#define FSA_IS_FILESYSTEM 26
struct IFS_IS_FILESYSTEM
{
      /* This is set to TRUE by the filesystem handler, if it is a filesystem
         and set to FALSE if it is not. */
    BOOL io_IsFilesystem;
};

/* Changes the number of buffers for the filesystem. The current number of
   buffers is returned. The size of the buffers is filesystem-dependend. */
#define FSA_MORE_CACHE 27
struct IFS_MORE_CACHE
{
      /* Number of buffers to add. May be negative to reduce number of buffers.
         This is to be set to the current number of buffers on success. */
    LONG io_NumBuffers;
};

/* Formats a volume, ie erases all data on it. */
#define FSA_FORMAT 28
struct IFS_FORMAT
{
    STRPTR io_VolumeName; /* New name for the volume. */
    ULONG  io_DosType;    /* New type for the volume. Filesystem specific. */
};

/* Resets/Reads the mount-mode of the volume passed in as io_Unit. The first
   and second argument work exactly like FSA_FILE_MODE, but the third
   argument can contain a password, if MMF_LOCKED is set. */
#define FSA_MOUNT_MODE 29
struct IFS_MOUNT_MODE
{
      /* The new mode to apply to the volume. See below for definitions. The
         filehandler fills this with the old mode bits. */
    ULONG  io_MountMode;
      /* This mask defines, which flags are to be changed. */
    ULONG  io_Mask;
      /* A passwort, which is needed, if MMF_LOCKED is set. */
    STRPTR io_Password;
};

/* The following actions are currently not supported. */
#if 0
#define FSA_SERIALIZE_DISK  30
#define FSA_FLUSH	    31
#define FSA_INHIBIT	    32
#define FSA_WRITE_PROTECT   33
#define FSA_DISK_CHANGE     34
#define FSA_ADD_NOTIFY	    35
#define FSA_REMOVE_NOTIFY   36
#define FSA_DISK_INFO	    37
#define FSA_CHANGE_SIGNAL   38
#define FSA_LOCK_RECORD     39
#define FSA_UNLOCK_RECORD   40
#endif

#define FSA_PARENT_DIR      41
#define FSA_PARENT_DIR_POST 42
struct IFS_PARENT_DIR
{
    /* this will contain the return value of the parent directory or
       NULL, if we are at the root directory already */
    char * io_DirName;
};

/*
    Allows us to change a console between raw and cooked mode.
*/
#define FSA_CONSOLE_MODE    43
struct IFS_CONSOLE_MODE
{
    LONG	io_ConsoleMode;
};
#define FCM_COOKED	0
#define FCM_RAW		1


/* io_FileMode for FSA_OPEN, FSA_OPEN_FILE and FSA_FILE_MODE. These are flags
   and may be or'ed. Note that not all filesystems support all flags. */
#define FMF_LOCK    (1L<<0) /* Lock exclusively. */
#define FMF_EXECUTE (1L<<1) /* Open for executing. */
/* At least one of the following two flags must be specified. Otherwise expect
   strange things to happen. */
#define FMF_WRITE   (1L<<2) /* Open for writing. */
#define FMF_READ    (1L<<3) /* Open for reading. */
#define FMF_CREATE  (1L<<4) /* Create file if it doesn't exist. */
#define FMF_CLEAR   (1L<<5) /* Truncate file on open. */
#define FMF_RAW     (1L<<6) /* Switch cooked to raw and vice versa. */

/* io_MountMode for FSA_MOUNT_MODE. These are flags and may be or'ed. */
#define MMF_READ	(1L<<0) /* Mounted for reading. */
#define MMF_WRITE	(1L<<1) /* Mounted for writing. */
#define MMF_READ_CACHE	(1L<<2) /* Read cache enabled. */
#define MMF_WRITE_CACHE (1L<<3) /* Write cache enabled. */
#define MMF_OFFLINE	(1L<<4) /* Filesystem currently does not use the
                                   device. */
#define MMF_LOCKED	(1L<<5) /* Mount mode is password protected. */


/* This structure is an extended IORequest (see <exec/io.h>). It is used for
   requesting actions from AROS filesystem handlers.
   Note that this structure may grow in the future. Do not depend on its size!
   You may use sizeof(struct IOFileSys) nevertheless if you are reserving
   memory for a struct IOFileSys as the size of it will never shrink. */
struct IOFileSys
{
    struct IORequest IOFS;	  /* Standard I/O request. */
    LONG	     io_DosError; /* Dos error code. */
    LONG             io_DirPos;   /* The result from telldir() is stored here */

    /* This union contains all the data needed for the various actions. */
    union
    {
/* Obsolete definition, included for backwards compatibility.*/
#if 0
        IPTR io_ArgArray[4]; /* Generic argument space. */
#endif

        struct {
            STRPTR io_DeviceName; /* Name of the device to open. */
            ULONG  io_Unit;       /* Number of unit to open. */
            IPTR * io_Environ;    /* Pointer to environment array. (see
                                     <dos/filehandler.h> */
        } io_OpenDevice;

	struct {
	    STRPTR io_Filename;
	} io_NamedFile;

        struct IFS_OPEN            io_OPEN;           /* FSA_OPEN */
        struct IFS_READ_WRITE      io_READ_WRITE;     /* FSA_READ, FSA_WRITE */
#define io_READ  io_READ_WRITE
#define io_WRITE io_READ_WRITE
        struct IFS_SEEK            io_SEEK;           /* FSA_SEEK */
#define io_SET_FILE_SIZE io_SEEK
        struct IFS_WAIT_CHAR       io_WAIT_CHAR;      /* FSA_WAIT_CHAR */
        struct IFS_FILE_MODE       io_FILE_MODE;      /* FSA_FILE_MODE */
        struct IFS_IS_INTERACTIVE  io_IS_INTERACTIVE; /* FSA_IS_INTERACTIVE */
        struct IFS_SAME_LOCK       io_SAME_LOCK;      /* FSA_SAME_LOCK */
        struct IFS_EXAMINE         io_EXAMINE;        /* FSA_EXAMINE */
#define io_EXAMINE_ALL io_EXAMINE
	struct IFS_EXAMINE_NEXT	   io_EXAMINE_NEXT;   /* FSA_EXAMINE_NEXT */
        struct IFS_OPEN_FILE       io_OPEN_FILE;      /* FSA_OPEN_FILE */
        struct IFS_CREATE_DIR      io_CREATE_DIR;     /* FSA_CREATE_DIR */
        struct IFS_CREATE_HARDLINK io_CREATE_HARDLINK;/* FSA_CREATE_HARDLINK */
        struct IFS_CREATE_SOFTLINK io_CREATE_SOFTLINK;/* FSA_CREATE_SOFTLINK */
        struct IFS_RENAME          io_RENAME;         /* FSA_RENAME */
        struct IFS_READ_SOFTLINK   io_READ_SOFTLINK;  /* FSA_READ_SOFTLINK */
        struct IFS_DELETE_OBJECT   io_DELETE_OBJECT;  /* FSA_DELETE_OBJECT */
        struct IFS_SET_COMMENT     io_SET_COMMENT;    /* FSA_SET_COMMENT */
        struct IFS_SET_PROTECT     io_SET_PROTECT;    /* FSA_SET_PROTECT */
        struct IFS_SET_OWNER       io_SET_OWNER;      /* FSA_SET_OWNER */
        struct IFS_SET_DATE        io_SET_DATE;       /* FSA_SET_DATE */
        struct IFS_IS_FILESYSTEM   io_IS_FILESYSTEM;  /* FSA_IS_FILESYSTEM */
        struct IFS_MORE_CACHE      io_MORE_CACHE;     /* FSA_MORE_CACHE */
        struct IFS_FORMAT          io_FORMAT;         /* FSA_FORMAT */
        struct IFS_MOUNT_MODE      io_MOUNT_MODE;     /* FSA_MOUNT_MODE */
        struct IFS_PARENT_DIR      io_PARENT_DIR;     /* FSA_PARENT_DIR */
	struct IFS_CONSOLE_MODE	   io_CONSOLE_MODE;   /* FSA_CONSOLE_MODE */
    } io_Union;
};
#if 0
    #define io_Args io_Union.io_ArgArray
#endif

#endif /* DOS_FILESYSTEM_H */
