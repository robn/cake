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

#include <string.h>

#include <proto/utility.h>

#include "private.h"

void *ExportBlock(struct MUIP_TextEditor_ExportBlock *msg, struct InstData *data)
{
  struct line_node *node;
  struct Hook *exportHook = data->ExportHook;
  ULONG wraplen = data->ExportWrap;
  struct ExportMessage emsg;
  struct marking newblock;
  ULONG flags = msg->flags;
  void *user_data = NULL;

  ENTER();

  // get information about marked text
  if(data->blockinfo.enabled)
    NiceBlock(&data->blockinfo, &newblock);
  else
  {
    newblock.startline = data->actualline;
    newblock.startx    = data->CPos_X;
    newblock.stopline  = data->actualline;
    newblock.stopx     = data->CPos_X;
  }

  if(isFlagSet(flags, MUIF_TextEditor_ExportBlock_TakeBlock))
  {

    if(msg->starty <= (ULONG)data->totallines)
      newblock.startline = LineNode(msg->starty+1, data);

    if(msg->startx <= (newblock.startline)->line.Length)
      newblock.startx = msg->startx;

    if(msg->stopx <= (newblock.startline)->line.Length)
      newblock.stopx = msg->stopx;

    if(msg->starty <= (ULONG)data->totallines)
      newblock.stopline = LineNode(msg->stopy+1, data);
  }

  node = newblock.startline;

  // clear the export message
  memset(&emsg, 0, sizeof(emsg));

  // now we export all selected/marked lines with
  // the currently active export hook
  while(node != NULL)
  {
    struct line_node *next_node = node->next;

    emsg.UserData = user_data;
    emsg.Contents = node->line.Contents;
    emsg.Length = node->line.Length;
    emsg.Styles = node->line.Styles;
    emsg.Colors = node->line.Colors;
    emsg.Highlight = node->line.Color;
    emsg.Flow = node->line.Flow;
    emsg.Separator = node->line.Separator;
    emsg.ExportWrap = wraplen;
    emsg.Last = next_node == NULL || node == newblock.stopline;

    // to make sure that for the last line we don't export the additional,
    // artificial newline '\n' we reduce the passed length value by one.
    if(next_node == NULL && emsg.Contents[node->line.Length-1] == '\n')
      emsg.Length--;

    // see if we have to skip some chars at the front or
    // back of the current line node
    if(isFlagClear(flags, MUIF_TextEditor_ExportBlock_FullLines))
    {
      if(node == newblock.startline)
        emsg.SkipFront = newblock.startx;
      else
        emsg.SkipFront = 0;

      if(node == newblock.stopline)
        emsg.SkipBack = emsg.Length-newblock.stopx;
      else
        emsg.SkipBack = 0;
    }
    
    // call the ExportHook and exit immediately if it returns NULL
    if((user_data = (void*)CallHookPkt(exportHook, NULL, &emsg)) == NULL)
      break;

    // check if the current node was the last one which
    // was marked, and if so we go and stop our export here
    if(node == newblock.stopline)
      break;

    node = next_node;
  }

  RETURN(user_data);
  return user_data;
}

