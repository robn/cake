#ifndef  GAMEPORT_INTERN_H
#define  GAMEPORT_INTERN_H

#include <exec/types.h>
#include <exec/devices.h>
#include <exec/semaphores.h>
#include <exec/interrupts.h>
#include <exec/devices.h>


/* Must always be a multiple of 3 since one event consists of code, x and y */
#define GP_NUMELEMENTS (2 * 3)
#define GP_BUFFERSIZE  (sizeof (UWORD) * GP_NUMELEMENTS) 

struct GameportBase
{
    struct Device      gp_device;
    struct ExecBase   *gp_sysBase;
    struct Library    *gp_LowLevelBase;

    BPTR               gp_seglist;
    
    struct MinList          gp_PendingQueue;  /* IOrequests (GPD_READEVENT) not done quick */
    struct SignalSemaphore  gp_QueueLock;

    struct Interrupt  gp_Interrupt;     /* Interrupt to invoke in case of keypress (or
					   releases) and there are pending requests */

    UWORD  *gp_eventBuffer;
    UWORD   gp_writePos;
    
    Object	*gp_Hidd;	/* Hidd object to use */
    struct Library *gp_OOPBase;
    
};


typedef struct GPUnit
{
    UWORD  gpu_readPos;		/* Position in the key buffer */
    UWORD  gpu_Qualifiers;      /* Known qualifiers at this moment */

    UBYTE  gpu_flags;           /* For unit flags definitions, see below */
} GPUnit;

#define GBUB_PENDING 0		/* Unit has pending request for gameport events */

#define GBUF_PENDING 0x01


#define expunge() \
AROS_LC0(BPTR, expunge, struct GameportBase *, GPBase, 3, Gameport)


#ifdef SysBase
#undef SysBase
#endif
#define SysBase GPBase->gp_sysBase

#ifdef OOPBase
#undef OOPBase
#endif
#define OOPBase GPBase->gp_OOPBase

#endif /* GAMEPORT_INTERN_H */

