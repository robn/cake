/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include <aros/libcall.h>
#include <proto/layers.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <exec/memory.h>
#include <graphics/rastport.h>
#include <graphics/clip.h>
#include "layers_intern.h"


/*****************************************************************************

    NAME */

	AROS_LH5(LONG, MoveSizeLayer,

/*  SYNOPSIS */
	AROS_LHA(struct Layer *, l , A0),
	AROS_LHA(LONG          , dx, D0),
	AROS_LHA(LONG          , dy, D1),
	AROS_LHA(LONG          , dw, D2),
	AROS_LHA(LONG          , dh, D3),

/*  LOCATION */
	struct LayersBase *, LayersBase, 30, Layers)

/*  FUNCTION
        Moves and resizes the layer in one step. Collects damage lists
        for those layers that become visible and are simple layers.
        If the layer to be moved is becoming larger the additional
        areas are added to a damagelist if it is a non-superbitmap
        layer. Refresh is also triggered for this layer.

    INPUTS
        l     - pointer to layer to be moved
        dx    - delta to add to current x position
        dy    - delta to add to current y position
        dw    - delta to add to current width
        dw    - delta to add to current height

    RESULT
        result - TRUE everyting went alright
                 FALSE an error occurred (out of memory)

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	27-11-96    digulla automatically created from
			    layers_lib.fd and clib/layers_protos.h

*****************************************************************************/
{
  AROS_LIBFUNC_INIT
  AROS_LIBBASE_EXT_DECL(struct LayersBase *,LayersBase)

  struct Layer * first, *_l, *lparent;
  struct Region * newshape = NewRegion(), * oldshape, r, rtmp, cutnewshape;
  struct Rectangle rectw, recth;

  rtmp.RegionRectangle = NULL; // min. initialization
  r.RegionRectangle = NULL; // min. initialization
  cutnewshape.RegionRectangle = NULL;
  
  LockLayers(l->LayerInfo);

  /*
   * First Create the new region of the layer:
   * adjust its size and position.
   */
   
  
  /* First create newshape with 0,0 origin, because l->shaperegion is in layer coords */
  
  newshape = NewRectRegion(0, 
                           0, 
                           l->bounds.MaxX - l->bounds.MinX + dw, 
                           l->bounds.MaxY - l->bounds.MinY + dh);
  if (l->shaperegion)
    AndRegionRegion(l->shaperegion, newshape);
  
  /* Now make newshape relative to old(!!) layer screen coords */
  _TranslateRect(&newshape->bounds, l->bounds.MinX+dx, l->bounds.MinY+dy);
  
  /* rectw and recth are now only needed for backfilling if layer got bigger -> see end of func */
  
  if (dw > 0)
  {
    rectw.MinX = dx+l->bounds.MaxX+1;
    rectw.MinY = dy+l->bounds.MinY;
    rectw.MaxX = dx+rectw.MinX + dw - 1;
    rectw.MaxY = dy+l->bounds.MaxY+dh;
  }

  if (dh > 0)
  {
    recth.MinX = dx+l->bounds.MinX;
    recth.MinY = dy+l->bounds.MaxY + 1;
    recth.MaxX = dx+l->bounds.MaxX+dw;
    recth.MaxY = dy+recth.MinY + dh - 1;
  }

  _SetRegion(newshape, &cutnewshape);
  AndRegionRegion(l->parent->visibleshape, &cutnewshape);

  first = GetFirstFamilyMember(l);
  /*
   * Must make a copy of the VisibleRegion of the first visible layer here
   * and NOT later!
   */
  _l = first;
  while (1)
  {
    if (IS_VISIBLE(_l))
    {
      _SetRegion(_l->VisibleRegion, &r);
      break;
    }
    
    if (l == _l)
      break;
      
    _l = _l->back;
  }
//kprintf("%s called for layer %p, first = %p!\n",__FUNCTION__,l,first);
  
  /*
   * First back up parts of layers that are behind the layer
   * family. Only need to do this if layer is moving or
   * getting bigger in size. Only need to visit those layers
   * that overlap with the new shape of the layer.
   */

        
  lparent = l->parent;
  _l = l->back;

#if 0
kprintf("\t\t%s: Backing up parts of layers that are behind the layer!\n",
        __FUNCTION__);
#endif
  while (1)
  {
    if (IS_VISIBLE(_l) && DO_OVERLAP(&cutnewshape.bounds, &_l->shape->bounds))
      _BackupPartsOfLayer(_l, &cutnewshape, 0, FALSE, LayersBase);
    else
      ClearRegionRegion(&cutnewshape, _l->VisibleRegion);

    if (_l == lparent)
    {
      if (IS_VISIBLE(_l) || (NULL == lparent->parent))
        break;
      else
        lparent = lparent->parent;
    }
    _l = _l->back;
  }

   _SetRegion(&cutnewshape, l->visibleshape);
  ClearRegion(&cutnewshape);

  /*
   * Now I need to move the layer and all its familiy to the new 
   * location.
   */
  oldshape = l->shape;
  l->shape = newshape;

  _l = l;
 
  while (1)
  {
    struct ClipRect * cr;

#if 0
kprintf("\t\t%s: BACKING up parts of THE LAYER TO BE MOVED!\n",
        __FUNCTION__);
#endif    

    if (1/* IS_VISIBLE(_l) */)
    {
      ClearRegion(_l->VisibleRegion);
      _BackupPartsOfLayer(_l, _l->shape, dx, TRUE, LayersBase);
    }
    /*
     * Effectively move the layer...
     */
    _TranslateRect(&_l->bounds, dx, dy);
     

    /*
     * ...and also its cliprects.
     */
    cr = _l->ClipRect;
    while (cr)
    {
      _TranslateRect(&cr->bounds, dx, dy);
      cr = cr->Next;
    }

    cr = _l->_cliprects;
    while (cr)
    {
      _TranslateRect(&cr->bounds, dx, dy);
      cr = cr->Next;
    }
    
    if (l != _l)   
    {
      _TranslateRect(&_l->shape->bounds, dx, dy);

      /*
       * Also calculate the visible shape!
       */
      _SetRegion(_l->shape, _l->visibleshape);
      AndRegionRegion(_l->parent->visibleshape, _l->visibleshape);
    }

    if (_l == first)
      break;
      
    _l = _l->front;
  }

  l->bounds.MaxX += dw;
  l->bounds.MaxY += dh;
  l->Width  += dw;
  l->Height += dh;


  /* 
   * Now make them visible again.
   */
  _l = first;

  while (1)
  {
    if (IS_VISIBLE(_l))
    {
#if 0
kprintf("\t\t%s: SHOWING parts of THE LAYER TO BE MOVED (children)!\n",
        __FUNCTION__);
#endif
      ClearRegion(_l->VisibleRegion);
      _ShowPartsOfLayer(_l, &r, LayersBase);
      
      if (l == _l)
        break;
      
      ClearRegionRegion(_l->visibleshape, &r);
    }

    if (l == _l)
      break;
      
    _l = _l->back;
  }


  /*
   * Now make those parts of the layers after l up to and including
   * its parent visible.
   */
  _SetRegion(l->VisibleRegion, &r);
  ClearRegionRegion(l->visibleshape, &r);
  _l = l->back;
  lparent = l->parent;
    
  while (1)
  {
#if 0
kprintf("\t\t%s: SHOWING parts of the layers behind the layer to be moved!\n",
        __FUNCTION__);
#endif
    if (IS_VISIBLE(_l) && 
       (  DO_OVERLAP(&l->visibleshape->bounds, &_l->shape->bounds) || 
          DO_OVERLAP(   &oldshape->bounds, &_l->shape->bounds) ))
    {
      ClearRegion(_l->VisibleRegion);
      _ShowPartsOfLayer(_l, &r, LayersBase);
    }
    else
      _SetRegion(&r, _l->VisibleRegion);

    if (IS_VISIBLE(_l) || IS_ROOTLAYER(_l))
      AndRegionRegion(_l->VisibleRegion, oldshape);

#if 0
    if (IS_ROOTLAYER(_l))
      kprintf("root reached! %p\n",_l);
#endif
      
    if (_l == lparent)
    {
      if (IS_VISIBLE(_l) || (NULL == lparent->parent))
        break;
      else
        lparent = lparent->parent;
    }
      
    if (IS_VISIBLE(_l))
      ClearRegionRegion(_l->visibleshape, &r);

    _l = _l->back;
  }

  ClearRegion(&rtmp);
    
  /*  
   * Now I need to clear the old layer at its previous place..
   * But I may only clear those parts where no layer has become
   * visible in the meantime.
   */
  if (!IS_EMPTYREGION(oldshape))
  {
    if (lparent &&
        (IS_SIMPLEREFRESH(lparent) || IS_ROOTLAYER(lparent)))
      _BackFillRegion(l->parent, oldshape, TRUE);
  }

  DisposeRegion(oldshape);

  /*
   * If the size of the layer became larger clear the
   * new area where it is visible.
   */
  if ((dw > 0 || dh > 0) && !IS_SUPERREFRESH(l))
  {
    ClearRegion(&r);
    if (dw > 0)
      OrRectRegion(&r, &rectw);
    
    if (dh > 0)
      OrRectRegion(&r, &recth);
    _BackFillRegion(l, &r, TRUE);
  }

  ClearRegion(&r);

  UnlockLayers(l->LayerInfo);

  return TRUE;

  AROS_LIBFUNC_EXIT
} /* MoveSizeLayer */
