#include <exec/types.h>

//#include <libraries/mui.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#ifdef _AROS
#include <proto/muimaster.h>
#endif
#include <clib/alib_protos.h>
#include <stdio.h>

#include <mui.h>
#undef SysBase

struct Library       *MUIMasterBase;

#ifndef _AROS

/* On AmigaOS we build a fake library base, because it's not compiled as sharedlibrary yet */
#include "muimaster_intern.h"

int openmuimaster(void)
{
    static struct MUIMasterBase_intern MUIMasterBase_instance;
    MUIMasterBase = (struct Library*)&MUIMasterBase_instance;

    MUIMasterBase_instance.sysbase = *((struct ExecBase **)4);
    MUIMasterBase_instance.dosbase = OpenLibrary("dos.library",37);
    MUIMasterBase_instance.utilitybase = OpenLibrary("utility.library",37);
    MUIMasterBase_instance.aslbase = OpenLibrary("asl.library",37);
    MUIMasterBase_instance.gfxbase = OpenLibrary("graphics.library",37);
    MUIMasterBase_instance.layersbase = OpenLibrary("layers.library",37);
    MUIMasterBase_instance.intuibase = OpenLibrary("intuition.library",37);
    MUIMasterBase_instance.cxbase = OpenLibrary("commodities.library",37);
    MUIMasterBase_instance.keymapbase = OpenLibrary("keymap.library",37);
    __zune_prefs_init(&__zprefs);

    return 1;
}

void closemuimaster(void)
{
}

#else

int openmuimaster(void)
{
    if ((MUIMasterBase = OpenLibrary("muimaster.library", 0))) return 1;
    return 0;
}

void closemuimaster(void)
{
    if (MUIMasterBase) CloseLibrary(MUIMasterBase);
}

#endif

ULONG xget(Object *obj, Tag attr)
{
  ULONG storage;
  GetAttr(attr, obj, &storage);
  return storage;
}

int main (int argc, char **argv)
{
    Object *app;
    Object *mainWin;

    if (!openmuimaster()) return 20;

    app = ApplicationObject,
	SubWindow, mainWin = WindowObject,
	    MUIA_Window_Title, "Input modes",
	    WindowContents, VGroup,
	        Child, SliderObject,
                   MUIA_Numeric_Value, 50,
                End,
            End,
        End,
    End;

    if (!app)
    {
	fprintf(stderr, "can't create application object.\n");
	goto error;
    }

    DoMethod(mainWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
	     app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

    set(mainWin, MUIA_Window_Open, TRUE);
    if (!xget(mainWin, MUIA_Window_Open))
    {
	MUI_DisposeObject(app);
	fprintf(stderr, "%s : can't open main window.\n", argv[0]);
	goto error;
    }

    {
	ULONG sigs = 0;

	while (DoMethod(app, MUIM_Application_NewInput, &sigs)
	       != MUIV_Application_ReturnID_Quit)
	{
	    if (sigs)
	    {
	        sigs = Wait(sigs | SIGBREAKF_CTRL_C);
	        if (sigs & SIGBREAKF_CTRL_C) break;
	    }
	}
    }
    
    set(mainWin, MUIA_Window_Open, FALSE);
    MUI_DisposeObject(app);

error:
    closemuimaster();

    return 0;
}
