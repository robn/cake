/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$

    Desc: Code for CONU_STANDARD console units.
    Lang: english
*/
#include <proto/graphics.h>
#include <proto/boopsi.h>
#include <proto/intuition.h>
#include <intuition/intuition.h>
#include <graphics/rastport.h>
#include <aros/asmcall.h>
#include <string.h>

#include "console_gcc.h"
#include "consoleif.h"

#define SDEBUG 1
#define DEBUG 1
#include <aros/debug.h>

#define CP_X(o) (CU(o)->cu_XROrigin + (CU(o)->cu_XCP * CU(o)->cu_XRSize))
#define CP_Y(o) (CU(o)->cu_YROrigin + (CU(o)->cu_YCP * CU(o)->cu_YRSize))


struct stdcondata
{
    struct DrawInfo *dri;
};


#undef ConsoleDevice
#define ConsoleDevice ((struct ConsoleBase *)cl->cl_UserData)


/********************
**  StdCon::New()  **
********************/
static Object *stdcon_new(Class *cl, Object *o, struct opSet *msg)
{
    EnterFunc(bug("StdCon::New()\n"));
    o = (Object *)DoSuperMethodA(cl, o, (Msg)msg);
    if (o)
    {
    	struct stdcondata *data = INST_DATA(cl, o);
	ULONG dispmid = OM_DISPOSE;
	/* Clear for checking inside dispose() whether stuff was allocated.
	   Basically this is bug-prevention.
	*/
	memset(data, 0, sizeof (struct stdcondata));
	
	
	data->dri = GetScreenDrawInfo(CU(o)->cu_Window->WScreen);
	if (data->dri)
	    ReturnPtr("StdCon::New", Object *, o);

    	CoerceMethodA(cl, o, (Msg)&dispmid);
    }
    ReturnPtr("StdCon::New", Object *, NULL);
    
}

/************************
**  StdCon::Dispose()  **
************************/
static VOID stdcon_dispose(Class *cl, Object *o, Msg msg)
{
    struct stdcondata *data= INST_DATA(cl, o);
    if (data->dri)
    	FreeScreenDrawInfo(CU(o)->cu_Window->WScreen, data->dri);
	
    /* Let superclass free its allocations */
    DoSuperMethodA(cl, o, msg);
    
    return;
}

/**************************
**  StdCon::DoCommand()  **
**************************/

VOID stdcon_docommand(Class *cl, Object *o, struct P_Console_DoCommand *msg)

{

    struct Window   *w  = CU(o)->cu_Window;
    struct RastPort *rp = w->RPort;
    UBYTE *params = msg->Params;
    struct stdcondata *data = INST_DATA(cl, o);
    
    EnterFunc(bug("StdCon::DoCommand(o=%p, cmd=%d, params=%p)\n",
    	o, msg->Command, params));
    switch (msg->Command)
    {
    case C_ASCII:
    	
    	D(bug("Writing char %c in win %s at (%d, %d)\n",
    		params[0], w->Title, CP_X(o), CP_Y(o) + rp->Font->tf_Baseline));
    		
    	SetAPen(rp, data->dri->dri_Pens[TEXTPEN]);
    	SetDrMd(rp, JAM1);
    	Move(rp, CP_X(o), CP_Y(o) + rp->Font->tf_Baseline);
    	Text(rp, &params[0], 1);
    	
    	Console_Right(o, COORD_WRITEPOS, 1);

    	break;

    case C_FORMFEED: {
    	/* Clear the console */

        UBYTE oldpen = rp->FgPen;
        
    	SetAPen( rp, data->dri->dri_Pens[ CU(o)->cu_BgPen ] );
    	RectFill(rp 
    		,CU(o)->cu_XROrigin
    		,CU(o)->cu_YROrigin
    		,CU(o)->cu_XRExtant
    		,CU(o)->cu_YRExtant);
    		
    	SetAPen(rp, oldpen);
    	
    } break;

    case C_BELL:
        /* !!! maybe trouble with LockLayers() here !!! */
//    	DisplayBeep(CU(o)->cu_Window->WScreen);
    	break;

    case C_BACKSPACE:
    	Console_Left(o, COORD_WRITEPOS, 1);
    	break;

    case C_HTAB:
    	break;
	
    case C_LINEFEED:
    	Console_Down(o, COORD_WRITEPOS, 1);
    	break;

/*    case C_VTAB:
    	break;
*/
    case C_CARRIAGE_RETURN:
    	/* Goto start of line */
    	CU(o)->cu_XCP = MIN_XCP;
    	Console_Down(o, COORD_WRITEPOS, 1);
    	break;


    case C_INDEX:
    	Console_Down(o, COORD_WRITEPOS, 1);
    	break;

    case C_NEXT_LINE:
    	CU(o)->cu_XCP = MIN_XCP;
    	Console_Down(o, COORD_WRITEPOS, 1);
    	break;

    case C_REVERSE_IDX:
    	Console_Up(o, COORD_WRITEPOS, 1);
    	break;

    case C_CURSOR_POS:
    	break;

    case C_CURSOR_HTAB:
    	break;

    case C_ERASE_IN_DISPLAY: {
    	UBYTE param = 1;
        UBYTE oldpen = rp->FgPen;
        
    	/* Clear till EOL */
    	Console_DoCommand(o, C_ERASE_IN_LINE, &param);
    	
    	/* Clear rest of area */
        
    	SetAPen( rp, data->dri->dri_Pens[ CU(o)->cu_BgPen ] );
    	RectFill(rp 
    		,CU(o)->cu_XROrigin
    		,CU(o)->cu_YROrigin + (CU(o)->cu_YCP + 1) * CU(o)->cu_YRSize
    		,CU(o)->cu_XRExtant
    		,CU(o)->cu_YRExtant);
    		
    	SetAPen(rp, oldpen);
    	
    	
    } break;

    case C_INSERT_LINE:
    	/* Scroll lines up */
//    	Scroll(unit, unit->cu_YCP, unit->cu_YMax, 1, ConsoleDevice);
    	break;

/*    case C_:
    	break;

    case C_:
    	break;

    case C_:
    	break;

    case C_:
    	break;
*/
    }

    ReturnVoid("StdCon::DoCommand");
}


AROS_UFH3(static IPTR, dispatch_stdconclass,
    AROS_UFHA(Class *,  cl,  A0),
    AROS_UFHA(Object *, o,   A2),
    AROS_UFHA(Msg,      msg, A1)
)
{
    IPTR retval = 0UL;
    
    switch (msg->MethodID)
    {
    case OM_NEW:
        retval = (IPTR)stdcon_new(cl, o, (struct opSet *)msg);
	break;
	
    case OM_DISPOSE:
    	stdcon_dispose(cl, o, msg);
	break;
	
    case M_Console_DoCommand:
    	stdcon_docommand(cl, o, (struct P_Console_DoCommand *)msg);
	break;
	
    default:
    	retval = DoSuperMethodA(cl, o, msg);
	break;
    }
    
    return retval;
    
}

#undef ConsoleDevice

Class *makestdconclass(struct ConsoleBase *ConsoleDevice)
{
    
   Class *cl;
   	
   cl = MakeClass(NULL, NULL ,CONSOLECLASSPTR , sizeof(struct stdcondata), 0UL);
   if (cl)
   {
    	cl->cl_Dispatcher.h_Entry = (APTR) AROS_ASMSYMNAME(dispatch_stdconclass);
    	cl->cl_Dispatcher.h_SubEntry = NULL;

    	cl->cl_UserData = (IPTR)ConsoleDevice;

   	return (cl);
    }
    return (NULL);
}

