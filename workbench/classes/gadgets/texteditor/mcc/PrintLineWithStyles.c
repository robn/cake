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

#include <graphics/text.h>
#include <clib/alib_protos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#include "private.h"

ULONG convert(ULONG style)
{
    unsigned long result = FS_NORMAL;

  if(style & BOLD)
    result |= FSF_BOLD;
  if(style & ITALIC)
    result |= FSF_ITALIC;
  if(style & UNDERLINE)
    result |= FSF_UNDERLINED;

  return(result);
}

ULONG ConvertPen (UWORD color, BOOL highlight, struct InstData *data)
{
  return(color ? (ULONG)(data->colormap ? (ULONG)data->colormap[color-1] : (ULONG)((color <= 8) ? _pens(data->object)[color-1] : color-9)) : (ULONG)(highlight ? (ULONG)data->highlightcolor : (ULONG)data->textcolor));
}

VOID DrawSeparator (struct RastPort *rp, WORD X, WORD Y, WORD Width, WORD Height, struct InstData *data)
{
  ENTER();

  if(Width > 3*Height)
  {
    SetAPen(rp, data->separatorshadow);
    RectFill(rp, X, Y, X+Width-2, Y);
    RectFill(rp, X, Y, X, Y+Height);

    SetAPen(rp, data->separatorshine);
    RectFill(rp, X+1, Y+Height, X+Width-1, Y+Height);
    RectFill(rp, X+Width-1, Y, X+Width-1, Y+Height);
  }

  LEAVE();
}

LONG PrintLine(LONG x, struct line_node *line, LONG line_nr, BOOL doublebuffer, struct InstData *data)
{
  STRPTR text = line->line.Contents;
  LONG length;
  struct RastPort *rp = &data->doublerp;

  ENTER();

  length  = LineCharsWidth(text+x, data);

  if(doublebuffer == FALSE)
    rp = &data->copyrp;

  if((line_nr > 0) && (data->update) && !(data->flags & FLG_Quiet) && data->rport != NULL && data->shown == TRUE)
  {
    LONG c_length = length;
    LONG startx = 0, stopx = 0;
    LONG starty = 0, xoffset = ((data->height-rp->TxBaseline+1)>>1)+1;
    LONG flow = 0;
    UWORD *styles = line->line.Styles;
    UWORD *colors = line->line.Colors;
    struct marking block;
    BOOL cursor = FALSE;

    if(line->line.Color && x == 0 && line->line.Length == 1)
      line->line.Color = FALSE;

    if(doublebuffer == FALSE)
    {
      starty = data->ypos+(data->height * (line_nr-1));
      xoffset = data->xpos;
    }

    flow = FlowSpace(line->line.Flow, text+x, data);
    Move(rp, xoffset+flow, starty+rp->TxBaseline);

    if(Enabled(data))
    {
      struct line_node *blkline;

      NiceBlock(&data->blockinfo, &block);

      blkline = block.startline->next;

      if(block.startline == block.stopline)
      {
        if(block.startline == line)
        {
          startx = block.startx;
          stopx = block.stopx;
        }
      }
      else
      {
        if(block.startline == line)
        {
          startx = block.startx;
          stopx = line->line.Length;
        }
        else
        {
          if(block.stopline == line)
          {
            stopx = block.stopx;
          }
          else
          {
            while((blkline != block.stopline) && (!stopx))
            {
              if(blkline == line)
              {
                stopx = line->line.Length;
              }
              blkline = blkline->next;
            }
          }
        }
      }
    }

    {
      UWORD blockstart = 0;
      UWORD blockwidth = 0;
      struct RastPort *old = muiRenderInfo(data->object)->mri_RastPort;

      if(startx < x+c_length && stopx > x)
      {
        if(startx > x)
          blockstart = TextLength(&data->tmprp, text+x, startx-x);
        else
          startx = x;

        blockwidth = ((stopx >= c_length+x) ? data->innerwidth-(blockstart+flow) : TextLength(&data->tmprp, text+startx, stopx-startx));
      }
      else if(!(data->flags & (FLG_ReadOnly | FLG_Ghosted)) &&
              line == data->actualline && data->CPos_X >= x &&
              data->CPos_X < x+c_length && !Enabled(data) &&
              (data->flags & FLG_Active || data->inactiveCursor == TRUE))
      {
        cursor = TRUE;
        blockstart = TextLength(&data->tmprp, text+x, data->CPos_X-x);

        // calculate the cursor width
        // if it is set to 6 then we should find out how the width of the current char is
        if(data->CursorWidth == 6)
          blockwidth = TextLength(&data->tmprp, (*(text+data->CPos_X) < ' ') ? (char *)" " : (char *)(text+data->CPos_X), 1);
        else
          blockwidth = data->CursorWidth;
      }

      SetDrMd(rp, JAM1);
      muiRenderInfo(data->object)->mri_RastPort = rp;

      // clear the background first
      DoMethod(data->object, MUIM_DrawBackground, xoffset, starty,
                                                  flow+blockstart, data->height,
                                                  (data->flags & FLG_InVGrp) ? 0 : data->xpos,
                                                  (data->flags & FLG_InVGrp) ? data->height*(data->visual_y+line_nr-2) : data->realypos+data->height * (data->visual_y+line_nr-2),
                                                  0);

      if(blockwidth)
      {
        ULONG color;

        // in case the gadget is in inactive state we use a different background
        // color for our selected area
        if((data->flags & FLG_Active) == 0 && (data->flags & FLG_Activated) == 0 &&
           (data->flags & FLG_ActiveOnClick) != 0)
        {
          color = data->inactivecolor;
        }
        else
          color = data->markedcolor;

        // if selectmode == 2 then a whole line should be drawn as being marked, so
        // we have to start at xoffset instead of xoffset+flow+blockstart.
        // Please note that the second part of the following "empiric" evaluation should
        // prevent that centered or right aligned lines are not correctly marked right
        // from the beginning of the line. However, it seems to be not cover 100% of all different
        // cases so that the evaluation if a line should be completely marked should be probably
        // moved elsewhere in future.
        if(data->selectmode == 2 ||
           (flow && data->selectmode != 1 && startx-x == 0 && cursor == FALSE &&
            ((data->blockinfo.startline != data->blockinfo.stopline) || x > 0)))
        {
          SetAPen(rp, color);
          RectFill(rp, xoffset, starty, xoffset+flow+blockwidth-1, starty+data->height-1);
        }
        else
        {
          SetAPen(rp, cursor ? data->cursorcolor : color);
          RectFill(rp, xoffset+flow+blockstart, starty, xoffset+flow+blockstart+blockwidth-1, starty+data->height-1);

          // if the gadget is in inactive state we just draw a skeleton cursor instead
          if(cursor == TRUE &&
             (data->flags & FLG_Active) == 0 && (data->flags & FLG_Activated) == 0)
          {
            DoMethod(data->object, MUIM_DrawBackground, xoffset+flow+blockstart+1, starty+1,
                                                        blockwidth-2, data->height-2,
                                                        (data->flags & FLG_InVGrp) ? 0 : data->xpos,
                                                        (data->flags & FLG_InVGrp) ? data->height*(data->visual_y+line_nr-2) : data->realypos+data->height * (data->visual_y+line_nr-2),
                                                        0);
          }
        }
      }


      {
        LONG  x_start = xoffset+blockstart+blockwidth,
            y_start = starty,
            x_width = data->innerwidth-(blockstart+blockwidth),
            y_width = data->height,
            x_ptrn = blockstart+blockwidth,
            y_ptrn = data->height*(data->visual_y+line_nr-2);

        if(blockwidth)
        {
          x_start += flow;
          x_width -= flow;
          x_ptrn += flow;
        }
        if(!(data->flags & FLG_InVGrp))
        {
          x_ptrn += data->xpos;
          y_ptrn += data->realypos;
        }

        DoMethod(data->object, MUIM_DrawBackground, x_start, y_start, x_width, y_width, x_ptrn, y_ptrn);
      }
      muiRenderInfo(data->object)->mri_RastPort = old;
    }

    if(doublebuffer == FALSE)
      AddClipping(data);

    SetAPen(rp, (line->line.Color ? data->highlightcolor : data->textcolor));

    while(c_length)
    {
      LONG p_length = c_length;

      SetSoftStyle(rp, convert(GetStyle(x, line)), AskSoftStyle(rp));
      if(styles)
      {
        while(*styles-1 <= x)
        {
          styles += 2;
        }
        if(*styles-x-1 < p_length)
        {
          p_length = *styles-x-1;
        }
      }

      if(colors)
      {
        while(*colors-1 <= x)
        {
          SetAPen(rp, ConvertPen(*(colors+1), line->line.Color, data));
          colors += 2;
        }
        if(*colors-x-1 < p_length)
        {
          p_length = *colors-x-1;
        }
      }

/*      if(stopx)
      {
        if((startx > x) && (startx-x < p_length))
        {
          p_length = startx-x;
          SetAPen(rp, 3);
        }
        else
        {
          if((stopx > x) && (stopx-x < p_length))
          {
            p_length = stopx-x;
            SetAPen(rp, 4);
          }
          else
            SetAPen(rp, (line->color ? data->highlightcolor : data->textcolor));
        }
      }
*/

      if(text[x+p_length-1] < ' ')
        Text(rp, text+x, p_length-1);
      else
        Text(rp, text+x, p_length);

      x += p_length;
      c_length -= p_length;
    }
    SetSoftStyle(rp, FS_NORMAL, AskSoftStyle(rp));

    if(line->line.Separator)
    {
        WORD  LeftX, LeftWidth,
            RightX, RightWidth,
            Y, Height;

      LeftX = xoffset;
      LeftWidth = flow-3;
      RightX = rp->cp_x+3;
      RightWidth = xoffset+data->innerwidth - RightX;
      Y = starty;
      Height = (line->line.Separator & LNSF_Thick) ? 2 : 1;

      if(line->line.Separator & LNSF_Middle)
        Y += (data->height/2)-Height;
      else
      {
        if(line->line.Separator & LNSF_Bottom)
          Y += data->height-(2*Height);
      }

      if(line->line.Separator & LNSF_StrikeThru || line->line.Length == 1)
      {
        LeftWidth = data->innerwidth;
      }
      else
      {
        DrawSeparator(rp, RightX, Y, RightWidth, Height, data);
      }
      DrawSeparator(rp, LeftX, Y, LeftWidth, Height, data);
    }


    if(data->flags & FLG_Ghosted)
    {
      UWORD *oldPattern = (UWORD *)rp->AreaPtrn;
      UBYTE oldSize = rp->AreaPtSz;
      UWORD newPattern[] = {0x1111, 0x4444};

      if(doublebuffer == TRUE)
      {
        ULONG ptrn1 = 0x11111111;
        ULONG ptrn2 = 0x44444444;

        ptrn1 = ptrn1>>((data->xpos-xoffset)%16);
        ptrn2 = ptrn2>>((data->xpos-xoffset)%16);

        if((data->height*(data->visual_y+line_nr-2))%2 == 0)
        {
          newPattern[0] = ptrn2;
          newPattern[1] = ptrn1;
        }
        else
        {
          newPattern[0] = ptrn1;
          newPattern[1] = ptrn2;
        }
      }
      else
      {
        if((data->height*(data->visual_y-1))%2 == 0)
        {
          newPattern[0] = 0x4444;
          newPattern[1] = 0x1111;
        }
      }

      SetDrMd(rp, JAM1);
      SetAPen(rp, *(_pens(data->object)+MPEN_SHADOW));
      rp->AreaPtrn = newPattern;
      rp->AreaPtSz = 1;
      RectFill(rp, xoffset, starty, xoffset+data->innerwidth-1, starty+data->height-1);
      rp->AreaPtrn = oldPattern;
      rp->AreaPtSz = oldSize;
    }

    if(doublebuffer == FALSE)
      RemoveClipping(data);
    else
    {
      if(line_nr == 1)
      {
        BltBitMapRastPort(rp->BitMap, xoffset, data->realypos-data->ypos, data->rport, data->xpos, data->realypos+(data->height * (line_nr-1)), data->innerwidth, data->height-(data->realypos-data->ypos), (ABC|ABNC));
      }
      else
      {
        if(line_nr == data->maxlines+1)
        {
          if(data->realypos != data->ypos)
          {
            BltBitMapRastPort(rp->BitMap, xoffset, 0, data->rport, data->xpos, data->ypos+(data->height * (line_nr-1)), data->innerwidth, data->realypos-data->ypos, (ABC|ABNC));
          }
        }
        else
        {
          BltBitMapRastPort(rp->BitMap, xoffset, 0, data->rport, data->xpos, data->ypos+(data->height * (line_nr-1)), data->innerwidth, data->height, (ABC|ABNC));
        }
      }
    }
  }

  if(data->flags & FLG_HScroll)
    length = line->line.Length;

  RETURN(length);
  return(length);
}
