/*
    $Id$
    $Log$
    Revision 1.1  1996/07/28 16:37:22  digulla
    Initial revision

    Desc:
    Lang: english
*/
#include <dos/dosextens.h>
#include <clib/exec_protos.h>
#include "dos_intern.h"

struct Process *AddProcess(struct Process *process, STRPTR argPtr,
ULONG argSize, APTR initialPC, APTR finalPC, struct DosLibrary *DOSBase)
{
    APTR *sp=process->pr_Task.tc_SPUpper;
    *--sp=SysBase;
    argSize=0;
    argPtr=NULL;
    process->pr_ReturnAddr=sp-1;
    process->pr_Task.tc_SPReg=(STRPTR)sp-SP_OFFSET;
    return (struct Process *)AddTask(&process->pr_Task,initialPC,finalPC);
} /* AddProcess */
