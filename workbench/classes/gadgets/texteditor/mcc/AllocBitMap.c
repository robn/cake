/***************************************************************************

 TextEditor.mcc - Textediting MUI Custom Class
 Copyright (C) 1997-2000 Allan Odgaard
 Copyright (C) 2005-2009 by TextEditor.mcc Open Source Team

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 TextEditor class Support Site:  http://www.sf.net/projects/texteditor-mcc

 $Id$

***************************************************************************/

#include <exec/memory.h>
#include <proto/graphics.h>
#include <proto/exec.h>

#include "SDI_compiler.h"
#include "Debug.h"

#include "private.h"

#if 0
#define USE_OS3 (1)
#else
#define USE_OS3 (((struct Library *)GfxBase)->lib_Version >= 39)
#endif

struct BitMap * SAVEDS ASM MUIG_AllocBitMap(REG(d0, LONG width), REG(d1, LONG height), REG(d2, LONG depth), REG(d3, LONG flags), REG(a0, struct BitMap *friend))
{
  struct BitMap *bm = NULL;

  ENTER();

  #if defined(__amigaos4__)
  bm = AllocBitMap(width,height,depth,flags,friend);
  #elif defined(__MORPHOS__) || defined(__AROS__)
  bm = AllocBitMap(width,height,depth,flags | BMF_MINPLANES | BMF_DISPLAYABLE, friend);
  #else
  if(USE_OS3)
  {
    if(friend != NULL)
    {
      if(FindSemaphore((char *)"cybergraphics.library") != NULL &&
         (GetBitMapAttr(friend,BMA_FLAGS) & BMF_INTERLEAVED) == 0)
        flags |= BMF_MINPLANES;
      else
        friend = NULL;
    }

    bm = AllocBitMap(width,height,depth,flags,friend);
  }
  else
  {
    if((bm = (struct BitMap *)AllocMem(sizeof(*bm), MEMF_CLEAR)) != NULL)
    {
      int i, plsize=RASSIZE(width,height);

      InitBitMap(bm,depth,width,height);

      if((bm->Planes[0] = AllocVec(plsize*depth,(flags & BMF_CLEAR) ? MEMF_CHIP|MEMF_CLEAR : MEMF_CHIP))) // !!!
      {
        for(i=1;i<depth;i++)
          bm->Planes[i] = (PLANEPTR)(((ULONG)bm->Planes[i-1]) + plsize);
      }
      else
      {
        FreeMem(bm,sizeof(*bm));
        bm = NULL;
      }
    }
  }
  #endif

  RETURN(bm);
  return bm;
}


VOID SAVEDS ASM MUIG_FreeBitMap(REG(a0, struct BitMap *bm))
{
  ENTER();

  #if defined(__amigaos4__) || defined(__MORPHOS__) || defined(__AROS__)
  FreeBitMap(bm);
  #else
  WaitBlit();  /* OCS/AGA need this before FreeBitMap() call */

  if(USE_OS3)
  {
    FreeBitMap(bm);
  }
  else
  {
    FreeVec(bm->Planes[0]);
    FreeMem(bm,sizeof(struct BitMap));
  }
  #endif

  LEAVE();
}
