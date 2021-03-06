/*
    Copyright � 1995-2008, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang:
*/
#include <exec/errors.h>
#include <exec/resident.h>
#include <exec/memory.h>
#include <libraries/expansion.h>
#include <proto/exec.h>
#include <proto/expansion.h>
#include <utility/tagitem.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include <proto/dos.h>
#include <aros/libcall.h>
#include <aros/symbolsets.h>

#include LC_LIBDEFS_FILE

#include <string.h>

static int OpenDev(LIBBASETYPEPTR nilbase, struct IOFileSys *iofs);

static int GM_UNIQUENAME(Init)(LIBBASETYPEPTR nilbase)
{
/*
 * Modules compiled with noexpunge always have seglist == NULL
 * The seglist is not kept as it is not needed because the module never
 * can be expunged
 if (GM_SEGLIST_FIELD(nilbase)==NULL) /* Are we a ROM module? * /
*/
    {
        struct DeviceNode *dn;
        /* Install NIL: handler into device list
         *
         * KLUDGE: The mountlists for NIL: should be into dos.library bootstrap routines.
         */

        if((dn = AllocMem(sizeof (struct DeviceNode) + 4 + AROS_BSTR_MEMSIZE4LEN(3),
                          MEMF_CLEAR|MEMF_PUBLIC)
           )
        )
        {
            struct IOFileSys dummyiofs;

            if (OpenDev(nilbase, &dummyiofs))
            {
                BSTR s = (BSTR)MKBADDR(((IPTR)dn + sizeof(struct DeviceNode) + 3) & ~3);

                ((struct Library *)nilbase)->lib_OpenCnt++;

                AROS_BSTR_putchar(s, 0, 'N');
                AROS_BSTR_putchar(s, 1, 'I');
                AROS_BSTR_putchar(s, 2, 'L');
                AROS_BSTR_setstrlen(s, 3);

                dn->dn_Type    = DLT_DEVICE;
                dn->dn_Ext.dn_AROS.dn_Unit    = dummyiofs.IOFS.io_Unit;
                dn->dn_Ext.dn_AROS.dn_Device  = dummyiofs.IOFS.io_Device;
                dn->dn_Handler = NULL;
                dn->dn_Startup = NULL;
                dn->dn_Name = s;
                dn->dn_Ext.dn_AROS.dn_DevName = AROS_BSTR_ADDR(dn->dn_Name);

                if (AddDosEntry((struct DosList *)dn))
                    return TRUE;
            }

            FreeMem(dn, sizeof (struct DeviceNode));
        }
    }
    /*
    else
        return TRUE;
     */
    
    return FALSE;
}

static int GM_UNIQUENAME(Open)
(
    LIBBASETYPEPTR nilbase,
    struct IOFileSys *iofs,
    ULONG unitnum,
    ULONG flags
)
{
    /* Mark Message as recently used. */
    iofs->IOFS.io_Message.mn_Node.ln_Type=NT_REPLYMSG;

    if (OpenDev(nilbase, iofs))
	return TRUE;

    iofs->IOFS.io_Error=IOERR_OPENFAIL;

    return FALSE;
}

static int OpenDev(LIBBASETYPEPTR nilbase, struct IOFileSys *iofs)
{
    ULONG *dev;
    
    dev=AllocMem(sizeof(ULONG),MEMF_PUBLIC|MEMF_CLEAR);
    if(dev!=NULL)
    {
        iofs->IOFS.io_Unit   = (struct Unit *)dev;
        iofs->IOFS.io_Device = (struct Device *)nilbase;
    	iofs->IOFS.io_Error = 0;
    	return TRUE;
    }
    else
    {
	iofs->io_DosError=ERROR_NO_FREE_STORE;
	return FALSE;
    }
}


static int GM_UNIQUENAME(Close)(LIBBASETYPEPTR nilbase, struct IOFileSys *iofs)
{
    ULONG *dev;

    dev=(ULONG *)iofs->IOFS.io_Unit;
    if(*dev)
    {
	iofs->io_DosError=ERROR_OBJECT_IN_USE;
	return FALSE;
    }

    /* Let any following attemps to use the device crash hard. */
    FreeMem(dev,sizeof(ULONG));
    iofs->io_DosError=0;

    return TRUE;
}

ADD2INITLIB(GM_UNIQUENAME(Init),0)
ADD2OPENDEV(GM_UNIQUENAME(Open),0)
ADD2CLOSEDEV(GM_UNIQUENAME(Close),0)

AROS_LH1(void, beginio,
 AROS_LHA(struct IOFileSys *, iofs, A1),
	   struct nilbase *, nilbase, 5, Nil)
{
    AROS_LIBFUNC_INIT
    LONG error=0;

    /*
	Do everything quick no matter what. This is possible
	because I never need to Wait().
    */
    switch(iofs->IOFS.io_Command)
    {
	case FSA_OPEN:
	case FSA_OPEN_FILE:
	    /* No names allowed on NIL: */
	    if (iofs->io_Union.io_NamedFile.io_Filename[0])
	    {
		error=ERROR_OBJECT_NOT_FOUND;
		break;
	    }
	    Forbid();
	    ++*(ULONG *)iofs->IOFS.io_Unit;
	    Permit();
	    break;

	case FSA_READ:
	    iofs->io_Union.io_READ.io_Length=0;
	    break;

	case FSA_WRITE:
	    break;

	case FSA_SEEK:
	    iofs->io_Union.io_SEEK.io_Offset = 0;
	    break;

	case FSA_CLOSE:
	    Forbid();
	    --*(ULONG *)iofs->IOFS.io_Unit;
	    Permit();
	    break;

	case FSA_IS_INTERACTIVE:
	    iofs->io_Union.io_IS_INTERACTIVE.io_IsInteractive = TRUE;
	    break;
	case FSA_SET_FILE_SIZE:
        case FSA_EXAMINE:
        case FSA_EXAMINE_NEXT:
        case FSA_EXAMINE_ALL:
        case FSA_CREATE_DIR:
        case FSA_CREATE_HARDLINK:
        case FSA_CREATE_SOFTLINK:
        case FSA_RENAME:
        case FSA_DELETE_OBJECT:
            error = ERROR_NOT_IMPLEMENTED;
            break;

	default:
	    error = ERROR_ACTION_NOT_KNOWN;
	    break;
    }

    /* Set error code */
    iofs->io_DosError=error;

    /* If the quick bit is not set send the message to the port */
    if(!(iofs->IOFS.io_Flags&IOF_QUICK))
	ReplyMsg(&iofs->IOFS.io_Message);

    AROS_LIBFUNC_EXIT
}

AROS_LH1(LONG, abortio,
 AROS_LHA(struct IOFileSys *, iofs, A1),
	   struct nilbase *, nilbase, 6, Nil)
{
    AROS_LIBFUNC_INIT
    /* Everything already done. */
    return 0;
    AROS_LIBFUNC_EXIT
}
