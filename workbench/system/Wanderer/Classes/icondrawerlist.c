/*
Copyright  2002-2008, The AROS Development Team. All rights reserved.
$Id$
*/

#include <aros/debug.h>

#define DEBUG_ILC_EVENTS
#define DEBUG_ILC_KEYEVENTS
#define DEBUG_ILC_ICONRENDERING
#define DEBUG_ILC_ICONSORTING
#define DEBUG_ILC_ICONSORTING_DUMP
#define DEBUG_ILC_ICONPOSITIONING
#define DEBUG_ILC_LASSO
#define DEBUG_ILC_MEMALLOC

//#define CREATE_FULL_DRAGIMAGE

#if !defined(__AROS__)
#define DRAWICONSTATE DrawIconState
#else
#define DRAWICONSTATE DrawIconStateA
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <dos/dos.h>
#include <dos/datetime.h>
#include <dos/filehandler.h>

#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/view.h>
#include <graphics/rpattr.h>
#include <workbench/icon.h>
#include <workbench/workbench.h>
#include <devices/rawkeycodes.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/icon.h>
#include <proto/layers.h>
#include <proto/muimaster.h>
#include <proto/dos.h>
#include <proto/iffparse.h>
#include <prefs/prefhdr.h>
#include <prefs/wanderer.h>
#include <proto/cybergraphics.h>

#include <cybergraphx/cybergraphics.h>

#include <libraries/mui.h>
//#include "muimaster_intern.h"
//#include "support.h"
//#include "imspec.h"
#include "iconlist_attributes.h"
#include "iconlist.h"
#include "icondrawerlist_private.h"

extern struct Library *MUIMasterBase;


/**************************************************************************
Read icons in
**************************************************************************/
static int IconDrawerList__ParseContents(struct IClass *CLASS, Object *obj)
{
	struct IconDrawerList_DATA *data = INST_DATA(CLASS, obj);
	BPTR                      lock = NULL, tmplock = NULL;
	char                      filename[256];
	char                      namebuffer[512];
	ULONG                     list_DisplayFlags = 0;

D(bug("[IconList]: %s()\n", __PRETTY_FUNCTION__));

	if (!data->drawer) return 1;

	lock = Lock(data->drawer, SHARED_LOCK);

	if (lock)
	{
		struct FileInfoBlock *fib = AllocDosObject(DOS_FIB, NULL);
		if (fib)
		{
			if (Examine(lock, fib))
			{
				GET(obj, MUIA_IconList_DisplayFlags, &list_DisplayFlags);
D(bug("[IconList] %s: DisplayFlags = 0x%p\n", __PRETTY_FUNCTION__, list_DisplayFlags));

				while(ExNext(lock, fib))
				{
					int len = strlen(fib->fib_FileName);
					memset(namebuffer, 0, 512);
					strcpy(filename, fib->fib_FileName);

D(bug("[IconList] %s: '%s', len = %d\n", __PRETTY_FUNCTION__, filename, len));

					if (len >= 5)
					{
						if (!Stricmp(&filename[len-5],".info"))
						{
							/* Its a .info file .. skip "disk.info" and just ".info" files*/
							if ((len == 5) || ((len == 9) && (!Strnicmp(filename, "Disk", 4))))
							{
D(bug("[IconList] %s: Skiping file named disk.info or just .info ('%s')\n", __PRETTY_FUNCTION__, filename));
								continue;
							}

							strcpy(namebuffer, data->drawer);
							memset((filename + len - 5), 0, 1); //Remove the .info section
							AddPart(namebuffer, filename, sizeof(namebuffer));
D(bug("[IconList] %s: Checking for .info files real file '%s'\n", __PRETTY_FUNCTION__, namebuffer));

							if ((tmplock = Lock(namebuffer, SHARED_LOCK)))
							{
								/* We have a real file so skip it for now and let it be found seperately */
D(bug("[IconList] %s: File found .. skipping\n", __PRETTY_FUNCTION__));
								UnLock(tmplock); 
								continue;
							}
						}
					}

D(bug("[IconList] %s: Registering file '%s'\n", __PRETTY_FUNCTION__, filename));
					strcpy(namebuffer, data->drawer);
					AddPart(namebuffer, filename, sizeof(namebuffer));

					struct IconEntry *this_Icon = NULL;
						
					if ((this_Icon = (struct IconEntry *)DoMethod(obj, MUIM_IconList_CreateEntry, (IPTR)namebuffer, (IPTR)filename, (IPTR)fib, (IPTR)NULL)))
					{
D(bug("[IconList] %s: Icon entry allocated @ 0x%p\n", __PRETTY_FUNCTION__, this_Icon));

						sprintf(namebuffer + strlen(namebuffer), ".info");
						if ((tmplock = Lock(namebuffer, SHARED_LOCK)))
						{
D(bug("[IconList] %s: File has a .info file .. updating info\n", __PRETTY_FUNCTION__));
							UnLock(tmplock); 
							if (!(this_Icon->ile_Flags & ICONENTRY_FLAG_HASICON)) 
								this_Icon->ile_Flags |= ICONENTRY_FLAG_HASICON;
						}

						if (list_DisplayFlags & ICONLIST_DISP_SHOWINFO)
						{
							if ((this_Icon->ile_Flags & ICONENTRY_FLAG_HASICON) && !(this_Icon->ile_Flags & ICONENTRY_FLAG_VISIBLE))
								this_Icon->ile_Flags |= ICONENTRY_FLAG_VISIBLE;
						}
						else if (!(this_Icon->ile_Flags & ICONENTRY_FLAG_VISIBLE))
						{
							this_Icon->ile_Flags |= ICONENTRY_FLAG_VISIBLE;
						}
						this_Icon->ile_IconNode.ln_Pri = 0;
					}
					else
					{
D(bug("[IconList] %s: Failed to Register file!!!\n", __PRETTY_FUNCTION__));
					}
				}
			}
	
			FreeDosObject(DOS_FIB, fib);
		}
	
		UnLock(lock);
	}

	return 1;
}

/**************************************************************************
OM_NEW
**************************************************************************/
IPTR IconDrawerList__OM_NEW(struct IClass *CLASS, Object *obj, struct opSet *message)
{
	struct IconDrawerList_DATA   *data = NULL;
	struct TagItem  	        *tag = NULL,
								*tags = NULL;

D(bug("[IconDrawerList]: %s()\n", __PRETTY_FUNCTION__));

	obj = (Object *)DoSuperNewTags(CLASS, obj, NULL,
		TAG_MORE, (IPTR) message->ops_AttrList);

	if (!obj) return FALSE;

	data = INST_DATA(CLASS, obj);

	/* parse initial taglist */
	for (tags = message->ops_AttrList; (tag = NextTagItem((const struct TagItem **)&tags)); )
	{
		switch (tag->ti_Tag)
		{
			case    MUIA_IconDrawerList_Drawer:
			data->drawer = StrDup((char *)tag->ti_Data);
			break;
		}
	}

	return (IPTR)obj;
}

/**************************************************************************
OM_DISPOSE
**************************************************************************/
IPTR IconDrawerList__OM_DISPOSE(struct IClass *CLASS, Object *obj, Msg message)
{
	struct IconDrawerList_DATA *data = INST_DATA(CLASS, obj);

D(bug("[IconDrawerList]: %s()\n", __PRETTY_FUNCTION__));
	
	if (data->drawer)
	{
D(bug("[IconDrawerList] %s: Freeing DIR name storage for '%s'\n", __PRETTY_FUNCTION__, data->drawer));

		FreeVec(data->drawer);
	}

	return DoSuperMethodA(CLASS, obj, message);
}

/**************************************************************************
OM_SET
**************************************************************************/
IPTR IconDrawerList__OM_SET(struct IClass *CLASS, Object *obj, struct opSet *message)
{
	struct IconDrawerList_DATA   *data = INST_DATA(CLASS, obj);
	struct TagItem  	        *tag = NULL,
								*tags = NULL;

D(bug("[IconDrawerList]: %s()\n", __PRETTY_FUNCTION__));

	/* parse initial taglist */
	for (tags = message->ops_AttrList; (tag = NextTagItem((const struct TagItem **)&tags)); )
	{
		switch (tag->ti_Tag)
		{
			case    MUIA_IconDrawerList_Drawer:
				if (data->drawer) FreeVec(data->drawer);

				data->drawer = StrDup((char*)tag->ti_Data);
				DoMethod(obj, MUIM_IconList_Update);

				break;
		}
	}

	return DoSuperMethodA(CLASS, obj, (Msg)message);
}

/**************************************************************************
OM_GET
**************************************************************************/
IPTR IconDrawerList__OM_GET(struct IClass *CLASS, Object *obj, struct opGet *message)
{
	/* small macro to simplify return value storage */
#define STORE *(message->opg_Storage)
	struct IconDrawerList_DATA *data = INST_DATA(CLASS, obj);

D(bug("[IconDrawerList]: %s()\n", __PRETTY_FUNCTION__));

	switch (message->opg_AttrID)
	{
		case MUIA_IconDrawerList_Drawer: STORE = (IPTR)data->drawer; return 1;
	}

	if (DoSuperMethodA(CLASS, obj, (Msg) message)) return 1;
	return 0;
#undef STORE
}

/**************************************************************************
MUIM_IconList_Update
**************************************************************************/
IPTR IconDrawerList__MUIM_IconList_Update(struct IClass *CLASS, Object *obj, struct MUIP_IconList_Update *message)
{
	struct IconDrawerList_DATA *data = INST_DATA(CLASS, obj);
	//struct IconEntry *node;

D(bug("[IconDrawerList]: %s()\n", __PRETTY_FUNCTION__));

	DoSuperMethodA(CLASS, obj, (Msg) message);

	DoMethod(obj, MUIM_IconList_Clear);

	/* If not in setup do nothing */
#warning "TODO: Handle MADF_SETUP"
//	if (!(_flags(obj) & MADF_SETUP)) return 1;

	IconDrawerList__ParseContents(CLASS, obj);

	/*_Sort takes care of icon placement and redrawing for us*/
	DoMethod(obj, MUIM_IconList_Sort);

	return 1;
}

#if WANDERER_BUILTIN_ICONDRAWERLIST
BOOPSI_DISPATCHER(IPTR, IconDrawerList_Dispatcher, CLASS, obj, message)
{
	switch (message->MethodID)
	{
		case OM_NEW: return IconDrawerList__OM_NEW(CLASS, obj, (struct opSet *)message);
		case OM_DISPOSE: return IconDrawerList__OM_DISPOSE(CLASS, obj, message);
		case OM_SET: return IconDrawerList__OM_SET(CLASS, obj, (struct opSet *)message);
		case OM_GET: return IconDrawerList__OM_GET(CLASS, obj, (struct opGet *)message);
		case MUIM_IconList_Update: return IconDrawerList__MUIM_Update(CLASS, obj, (APTR)message);
	}
	return DoSuperMethodA(CLASS, obj, message);
}
BOOPSI_DISPATCHER_END

/* Class descriptor. */
const struct __MUIBuiltinClass _MUI_IconDrawerList_desc = { 
	MUIC_IconDrawerList, 
	MUIC_IconList, 
	sizeof(struct IconDrawerList_DATA), 
	(void*)IconDrawerList_Dispatcher 
};
#endif