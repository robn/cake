/*
    (C) 1995-2000 AROS - The Amiga Research OS
    $Id$

    Desc: Support functions for the colorwheel class
    Lang: English
    
    :ts=8
*/

#include <exec/memory.h>
#include <graphics/gfxmacros.h>
#include <intuition/classes.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/colorwheel.h>
#include <proto/cybergraphics.h>
#ifdef _AROS
#include <cybergraphx/cybergraphics.h>
#else
#include <cybergraphics/cybergraphics.h>
#endif

#include <gadgets/colorwheel.h>

#include "colorwheel_intern.h"

#ifndef _AROS
#include "bmbmrp.h"
#endif

#if FIXED_MATH
#include "fixmath.h"
#else
#include <math.h>
#endif

/***************************************************************************************************/

#ifdef _AROS
#define SDEBUG 0
#define DEBUG 0
#include <aros/debug.h>
#endif

#if FIXED_MATH
#define SQR(x)			FixMul( (x), (x) )
#else
#define SQR(x) 			((x) * (x))
#endif

#define CW_PI 			3.14159265358979

#define USE_WRITEPIXELARRAY 	1
#define USE_SYMMETRIC_SPEEDUP   1

#define MAKE_RGB_BE(r,g,b)	( (((r) >>  8) & 0x00FF0000) | \
				  (((g) >> 16) & 0x0000FF00) | \
				  (((b) >> 24) & 0x000000FF) )
				  
#define MAKE_RGB_LE(r,g,b)	( (((r) >> 16) & 0x0000FF00) | \
				  (((g) >>  8) & 0x00FF0000) | \
				  (((b) >>  0) & 0xFF000000) )

#ifdef _AROS
#if !AROS_BIG_ENDIAN
#   define MAKE_RGB(r,g,b) 	MAKE_RGB_LE(r,g,b)
#else
#   define MAKE_RGB(r,g,b) 	MAKE_RGB_BE(r,g,b)
#endif
#else
#   define MAKE_RGB(r,g,b) 	MAKE_RGB_BE(r,g,b)
#endif

/****************************************************************************/

UBYTE Bayer16[16][16] =
{
   {   1,235, 59,219, 15,231, 55,215,  2,232, 56,216, 12,228, 52,212},
   { 129, 65,187,123,143, 79,183,119,130, 66,184,120,140, 76,180,116},
   {  33,193, 17,251, 47,207, 31,247, 34,194, 18,248, 44,204, 28,244},
   { 161, 97,145, 81,175,111,159, 95,162, 98,146, 82,172,108,156, 92},
   {   9,225, 49,209,  5,239, 63,223, 10,226, 50,210,  6,236, 60,220},
   { 137, 73,177,113,133, 69,191,127,138, 74,178,114,134, 70,188,124},
   {  41,201, 25,241, 37,197, 21,254, 42,202, 26,242, 38,198, 22,252},
   { 169,105,153, 89,165,101,149, 85,170,106,154, 90,166,102,150, 86},
   {   3,233, 57,217, 13,229, 53,213,  0,234, 58,218, 14,230, 54,214},
   { 131, 67,185,121,141, 77,181,117,128, 64,186,122,142, 78,182,118},
   {  35,195, 19,249, 45,205, 29,245, 32,192, 16,250, 46,206, 30,246},
   { 163, 99,147, 83,173,109,157, 93,160, 96,144, 80,174,110,158, 94},
   {  11,227, 51,211,  7,237, 61,221,  8,224, 48,208,  4,238, 62,222},
   { 139, 75,179,115,135, 71,189,125,136, 72,176,112,132, 68,190,126},
   {  43,203, 27,243, 39,199, 23,253, 40,200, 24,240, 36,196, 20,254},
   { 171,107,155, 91,167,103,151, 87,168,104,152, 88,164,100,148, 84}
};

/***************************************************************************************************/

#if FIXED_MATH
BOOL CalcWheelColor(LONG x, LONG y, LONG cx, LONG cy, ULONG *hue, ULONG *sat)
{
    Fixed32	r, l, h, s, sinus;
    LONG	rx, ry;

    rx = cx - x;
    ry = ( y - cy ) * cx / cy;
    
    r  = FixSqrti( (rx*rx) + (ry*ry) );
    
    h = (r != 0) ? FixAtan2( FixDiv( INT_TO_FIXED(rx), r ), FixDiv( INT_TO_FIXED(ry), r ) ) : 0;

    cx = INT_TO_FIXED( cx );

    l = FixSqrt( FixSqr( FixMul( cx, FixSinCos( h + (FIXED_PI/2), &sinus ))) +
    	         FixSqr( FixMul( cx, sinus)) );
    
    s = FixDiv( r, l );
    h = FixDiv( h + FIXED_PI, FIXED_2PI );
    
    if (s == 0)
        s = 1;
    else if (s >= FIXED_ONE)
        s = FIXED_ONE-1;

    *hue = ( h << 16 ) | h;
    *sat = ( s << 16 ) | s;
    
    return (r <= cx);
}
#else
BOOL CalcWheelColor(LONG x, LONG y, double cx, double cy, ULONG *hue, ULONG *sat)
{
    double d, r, rx, ry, l, h, s;

#if 1
    /* Should also work with not perfect (cy == cy) circle */
    
    rx = (double) cx - x;
    ry = ((double) y - cy) * cx / cy;

    /* d = (SQR(cx) * SQR(rx) + SQR(cx) * SQR(ry) - SQR(cx) * SQR(cx)); */

    r = sqrt (SQR(rx) + SQR(ry));
    if (r > cx) d = 1.0; else d = 0.0;
    
    if (r != 0.0)
        h = atan2 (rx / r, ry / r);
    else
        h = 0.0;

    l = sqrt (SQR((cx * cos (h + 0.5 * CW_PI))) + SQR((cx * sin (h + 0.5 * CW_PI))));
    /*             ^^                                  ^^                          */
    /* no bug!                                                                     */
    
#else
    /* Does not work well if cx != cy (elliptical shape) */
    
    rx = (double) cx - x;
    ry = (double) y - cy;

    d = (SQR(cy) * SQR(rx) + SQR(cx) * SQR(ry) - SQR(cx) * SQR(cy));

    r = sqrt (SQR(rx) + SQR(ry));

    if (r != 0.0)
        h = atan2 (rx / r, ry / r);
    else
        h = 0.0;

    l = sqrt (SQR((cx * cos (h + 0.5 * CW_PI))) + SQR((cy * sin (h + 0.5 * CW_PI))));
#endif

    s = r / l;
    h = (h + CW_PI) / (2.0 * CW_PI);
    
    if (s == 0.0)
        s = 0.00001;
    else if (s > 1.0)
        s = 1.0;

    *hue = (ULONG)rint (h * 0xFFFFFFFF);
    *sat = (ULONG)rint (s * 0xFFFFFFFF);
    
    return (d > 0.0) ? FALSE : TRUE;
}
#endif
    
/***************************************************************************************************/

STATIC VOID CalcKnobPos(struct ColorWheelData *data, WORD *x, WORD *y)
{
#if FIXED_MATH
    Fixed32 alpha, sat, sinus;
   
    alpha = data->hsb.cw_Hue >> 16;
    alpha = FixMul( alpha, FIXED_2PI ) - (FIXED_PI/2);

    sat = data->hsb.cw_Saturation >> 16;

    *x = data->wheelcx + (WORD) FIXED_TO_INT( FixMul( ( (long) data->wheelrx ) * sat, FixSinCos(alpha,&sinus) ) );
    *y = data->wheelcy + (WORD) FIXED_TO_INT( FixMul( ( (long) data->wheelry ) * sat, sinus ) );
#else
    double alpha, sat;
   
    alpha = (double)data->hsb.cw_Hue  / (double) 0xFFFFFFFF;
    alpha *= CW_PI * 2.0;
    alpha -= CW_PI / 2.0;
   
    sat = (double)data->hsb.cw_Saturation / (double) 0xFFFFFFFF;
    
    *x = data->wheelcx + (WORD) ((double)data->wheelrx * sat * cos(alpha));
    *y = data->wheelcy + (WORD) ((double)data->wheelry * sat * sin(alpha));
#endif
}
                                                                         
/***************************************************************************************************/

STATIC VOID TrueWheel(struct ColorWheelData *data, struct RastPort *rp, struct IBox *box,
		      struct ColorWheelBase_intern *ColorWheelBase)
{
    struct ColorWheelHSB 	hsb;
    struct ColorWheelRGB 	rgb;
    ULONG			col;
    WORD 			x, y, left, top, width, height;
#if FIXED_MATH
    LONG			cx, cy;
#else
    double 			cx, cy;
#endif

    left   = box->Left;
    top    = box->Top;
    width  = box->Width;
    height = box->Height;
#if FIXED_MATH    
    cx = width / 2;
    cy = height / 2;
#else
    cx = (double)width  / 2.0;
    cy = (double)height / 2.0;
#endif
        
    hsb.cw_Brightness = 0xFFFFFFFF;
    
#if USE_WRITEPIXELARRAY
    if (!data->rgblinebuffer || data->rgblinebuffer_size < width)
    {
        if (data->rgblinebuffer) FreeVec(data->rgblinebuffer);
	
	data->rgblinebuffer = AllocVec(width * sizeof(LONG), MEMF_ANY);
	data->rgblinebuffer_size = width;
    }
#endif

    if (data->rgblinebuffer)
    {/*
        ULONG backrgb[3];
	ULONG backcol;
	
	GetRGB32(data->scr->ViewPort.ColorMap, 0, 1, backrgb);
	
	backcol = ((backrgb[0] >>  8) & 0xFF0000) |
		  ((backrgb[1] >> 16) & 0x00FF00) |
		  ((backrgb[2] >> 24) & 0x0000FF);
	*/	  
	for(y = 0; y < height; y++)
	{
	    WORD	 startX = 0, w = width;
	    ULONG	*p = data->rgblinebuffer;
	#if USE_SYMMETRIC_SPEEDUP
		ULONG	*p2 = &p[width];
            for(x = 0; x < width / 2; x++)
            
	#else	
            for(x = 0; x < width; x++)
	#endif
	    {
		if (CalcWheelColor(x, y, cx, cy, &hsb.cw_Hue, &hsb.cw_Saturation))
		{
	            ConvertHSBToRGB(&hsb, &rgb);
		    
		    col = MAKE_RGB(rgb.cw_Red, rgb.cw_Green, rgb.cw_Blue);
		    
		    *p++ = col;
		#if USE_SYMMETRIC_SPEEDUP
		    col = MAKE_RGB(rgb.cw_Red, rgb.cw_Blue, rgb.cw_Green);
		    *--p2 = col;
		#endif
		} else {
		    p++;
		    w--; startX++;
		#if USE_SYMMETRIC_SPEEDUP
		    w--; --p2;
		#endif
		}
		
	    } /* for(x = 0; x < width; x++) */
	    
	    if (w > 0) WritePixelArray(&data->rgblinebuffer[startX],
	    		    0,
			    0,
			    0, //width * sizeof(LONG),
			    rp,
			    startX+left,
			    top + y,
			    w,
			    1,
			    RECTFMT_ARGB);
			    
	} /* for(y = 0; y < height; y++) */
	
    } /* if (data->rgblinebuffer) */
    else
    {
    	for(y = 0; y < height; y++)
	{
	#if USE_SYMMETRIC_SPEEDUP
	    for(x = 0; x < width / 2; x++)
	#else
            for(x = 0; x < width; x++)
        #endif
	    {
		if (CalcWheelColor(x, y, cx, cy, &hsb.cw_Hue, &hsb.cw_Saturation))
		{
            		ConvertHSBToRGB(&hsb, &rgb);
            		
            		col = MAKE_RGB_BE(rgb.cw_Red, rgb.cw_Green, rgb.cw_Blue);
		    
		    	WriteRGBPixel(rp, left + x, top + y, col);
		    
			#if USE_SYMMETRIC_SPEEDUP
			    col = MAKE_RGB_BE(rgb.cw_Red, rgb.cw_Blue, rgb.cw_Green);
		    
			    WriteRGBPixel(rp, left + width - 1 - x, top + y , col);
			#endif			
		}
		
	    } /* for(x = 0; x < width; x++) */
	    
	} /* for(y = 0; y < height; y++) */
	
    } /* data->rgbinebuffer == NULL */
}

/****************************************************************************/

STATIC VOID ClutWheel(struct ColorWheelData *data, struct RastPort *rp, struct IBox *box,
		      struct ColorWheelBase_intern *ColorWheelBase)
{
    struct ColorWheelHSB 	hsb;
    struct ColorWheelRGB 	rgb;
    struct RastPort	       *tRP = &data->trp;
    struct BitMap	       *tBM;
    /* ULONG			col; */
    WORD 			x, y, left, top, width, height;
#if FIXED_MATH
    LONG			cx, cy;
#else
    double 			cx, cy;
#endif
    UBYTE			*buf;

    left   = box->Left;
    top    = box->Top;
    width  = box->Width;
    height = box->Height;

    if( data->pens == NULL )
    {
    	STRPTR		abbrv = data->abbrv;
    	PLANEPTR	ras;    	
    	UWORD		cx = data->wheelcx,
    			cy = data->wheelcy,
    			rx = data->wheelrx - 4,
    			ry = data->wheelry - 4,
    			depth = GetBitMapAttr( rp->BitMap, BMA_DEPTH );    	
    	ULONG		rasSize;

    	rasSize = RASSIZE( width*depth, height );
    	
    	if( (ras = AllocVec( rasSize, MEMF_CHIP ) ) )
    	{    		
    	    struct AreaInfo	ai;
	    struct TmpRas	tr;    		
    	    STATIC UWORD 	pattern[] = { 0xaaaa, 0x5555 };
    	    UWORD		black = FindColor( data->scr->ViewPort.ColorMap, 0,0,0, -1 ),
    			    	white = FindColor( data->scr->ViewPort.ColorMap, ~0,~0,~0, -1 ),
    			    	buf[10] = {};
    	    WORD		endx, endy, TxOffset;

	    InitArea( &ai, buf, sizeof( buf ) / 5 );
    	    rp->AreaInfo = &ai;
	    rp->TmpRas = &tr;
	    InitTmpRas( &tr, ras, rasSize );

	    SetAPen( rp, black );

	    AreaEllipse( rp, cx, cy, rx, ry );
	    AreaEnd( rp );

	    SetAPen( rp, white );
	    SetFont( rp, data->dri->dri_Font );

	    TxOffset = rp->TxHeight-rp->TxBaseline;

	    endx =  500*rx/2000, // 30�
    	    endy = -865*ry/2000;

    	    Move( rp, cx + rx/2, cy );
	    Draw( rp, cx + rx, cy );

	    Move( rp, cx - rx/2, cy );
	    Draw( rp, cx - rx, cy );

    	    Move( rp, cx + endx, cy + endy );
    	    Draw( rp, cx + 2*endx, cy + 2*endy );

    	    Move( rp, cx + endx, cy - endy );
    	    Draw( rp, cx + 2*endx, cy - 2*endy );

    	    Move( rp, cx - endx, cy + endy );
    	    Draw( rp, cx - 2*endx, cy + 2*endy );

    	    Move( rp, cx - endx, cy - endy );
    	    Draw( rp, cx - 2*endx, cy - 2*endy );    		

    	    endx =  866*rx/1500; // 60�
    	    endy = -499*ry/1500;

    	    Move( rp, cx + endx + ( TextLength( rp, abbrv, 1L ) / 2 ), cy - (endy - TxOffset) );
    	    Text( rp, abbrv++, 1L ); // G

    	    Move( rp, cx - ( TextLength( rp, abbrv, 1L ) / 2 ), cy + (ry - ry/4) + TxOffset );
    	    Text( rp, abbrv++, 1L ); // C

    	    Move( rp, cx - endx - ( TextLength( rp, abbrv, 1L ) ), cy - (endy - TxOffset) );
    	    Text( rp, abbrv++, 1L ); // B

    	    Move( rp, cx - endx - ( TextLength( rp, abbrv, 1L ) ), cy + (endy - TxOffset) );
    	    Text( rp, abbrv++, 1L ); // M

    	    Move( rp, cx - ( TextLength( rp, abbrv, 1L ) / 2 ), cy - (ry - ry/4) + TxOffset );
    	    Text( rp, abbrv++, 1L ); // R

    	    Move( rp, cx + endx + ( TextLength( rp, abbrv, 1L ) / 2 ), cy + (endy - TxOffset) );
    	    Text( rp, abbrv++, 1L ); // Y

	    SetAfPt( rp, pattern, 1L );

	    AreaEllipse( rp, cx, cy, rx/2, ry/2 );
	    AreaEnd( rp );

	    SetAfPt( rp, NULL, 0L );

	    AreaEllipse( rp, cx, cy, rx/5, ry/5 );
	    AreaEnd( rp );

	    WaitBlit();
	    FreeVec( ras );

	    rp->AreaInfo = NULL;
	    rp->TmpRas = NULL;	    	
   	}
   		
   	return;
    }	

#if FIXED_MATH    
    cx = width / 2;
    cy = height / 2;
#else
    cx = (double)width  / 2.0;
    cy = (double)height / 2.0;
#endif

    hsb.cw_Brightness = 0xFFFFFFFF;
    
    if( ( tBM = AllocBitMap( width,1, GetBitMapAttr( rp->BitMap, BMA_DEPTH ), 0L, NULL ) ) )
    {    	
    	tRP->BitMap = tBM;
    	
    	if( ( buf = AllocVec( (((width+15)>>4)<<4), MEMF_ANY ) ) )
    	{    		
    	    LONG range = data->range,
	    	 levels = data->levels;

	    for(y = 0; y < height; y++)
	    {
		WORD   startX = 0, w = width;
		UBYTE  *p = buf;
	    #if USE_SYMMETRIC_SPEEDUP
		UBYTE  *p2 = &p[width];
		for(x = 0; x < width / 2; x++)
	    #else
	        for(x = 0; x < width; x++)
	    #endif
		{
		    if (CalcWheelColor(x, y, cx, cy, &hsb.cw_Hue, &hsb.cw_Saturation))
		    {
			LONG	t,v, r,g,b, base;

		        ConvertHSBToRGB(&hsb, &rgb);

		        t = Bayer16[y & 15][x & 15];
			t = (t * range) / 255;

		        v = ( rgb.cw_Red >> 24 );

		        base = (v / range) * range;

		        r = (v - base > t) ? base + range : base;

		        v = ( rgb.cw_Green >> 24 );

		        base = (v / range) * range;

			g = (v - base > t) ? base + range : base;

		        v = ( rgb.cw_Blue >> 24 );

		        base = (v / range) * range;

		        b = (v - base > t) ? base + range : base;

	            	r = ((r / range) * levels * levels);
	            	g /= range;
	            	b /= range;
	            	base =  r + (g*levels) + b;	            	    
	            	*p++ = data->pens[ base ];

	            	#if USE_SYMMETRIC_SPEEDUP
	            	base = r + (b*levels) + g;
	            	*--p2 = data->pens[ base ];
	            	#endif
		    }
		    else
		    {
			startX++;
			w--; p++;
			#if USE_SYMMETRIC_SPEEDUP
			w--; --p2;
			#endif
		    }	

		} /* for(x = 0; x < width; x++) */

		if (w > 0) WritePixelLine8( rp, startX+left,top+y, w, &buf[startX], tRP );

	    } /* for(y = 0; y < height; y++) */

	    FreeVec( buf );
	}	
	
	WaitBlit();
	FreeBitMap( tBM );
    } 
}

/***************************************************************************************************/

VOID RenderWheel(struct ColorWheelData *data, struct RastPort *rp, struct IBox *box,
		 struct ColorWheelBase_intern *ColorWheelBase)
{
    struct IBox 	wbox;
    struct RastPort	temprp;
    WORD 		cx, cy, rx, ry;
    
    cx = data->frame ? BORDERWHEELSPACINGX * 4 : BORDERWHEELSPACINGX * 2;
    cy = data->frame ? BORDERWHEELSPACINGY * 4 : BORDERWHEELSPACINGY * 2;
    
    data->wheeldrawn = FALSE;
    
    if ( (box->Width < cx) || (box->Height < cy) ) return;
    
    if (!data->bm || (box->Width != data->bmwidth) || (box->Height != data->bmheight))
    {
        if (data->bm)
	{
	    WaitBlit();
	    
	    if (data->mask)
	    {	    	
	    	FreeVec( data->mask );
	    
	        data->mask = NULL;
   	    } 
	    
	    FreeBitMap(data->bm);
	}
	
        data->bm = AllocBitMap(box->Width,
		       	       box->Height,
		               GetBitMapAttr(rp->BitMap, BMA_DEPTH),
		               BMF_MINPLANES,
		               rp->BitMap);

	if (data->bm)
	{
	    data->bmwidth  = box->Width;
	    data->bmheight = box->Height;
	    
	    wbox.Left   = data->frame ? BORDERWHEELSPACINGX : 2;
	    wbox.Top    = data->frame ? BORDERWHEELSPACINGY : 2;
	    wbox.Width  = (box->Width  - (data->frame ? BORDERWHEELSPACINGX * 2 : 4)) & ~1;
	    wbox.Height = (box->Height - (data->frame ? BORDERWHEELSPACINGY * 2 : 4)) & ~1;
#if FIXED_MATH	 
	 	if( wbox.Width > 440 )
	 	{
	 		wbox.Left  += (wbox.Width-440)/2;
	 		wbox.Width  = 440;
	 	}
	 	
	 	if( wbox.Height > 440 )
	 	{
	 		wbox.Top    += (wbox.Height-440)/2;
	 		wbox.Height  = 440;
	 	}	
#endif
	    InitRastPort(&temprp);
	    temprp.BitMap = data->bm;	    
	       
	    SetDrMd(&temprp, JAM1);
	    SetRast( &temprp, data->dri->dri_Pens[ BACKGROUNDPEN ] );
	    
	    rx = wbox.Width / 2;
	    ry = wbox.Height / 2;

	    cx = wbox.Left + rx;
	    cy = wbox.Top + ry;

	    data->wheelcx = cx;
	    data->wheelcy = cy;
	    data->wheelrx = rx;
	    data->wheelry = ry;	    

	    if (data->frame)
	    {
		struct TagItem fitags[] =
		{
		    {IA_Width	, box->Width	},
		    {IA_Height	, box->Height	},
		    {TAG_DONE			}
		};
				
		SetAttrsA(data->frame, fitags);
		DrawImageState(&temprp, (struct Image *)data->frame, 0, 0, IDS_NORMAL, data->dri);
            }
            else
            {
            	struct BitMap	maskBM;
            	ULONG		bmWidth, bmHeight, rasSize;            	
            	PLANEPTR	ras;
            	
         	bmWidth = GetBitMapAttr( data->bm, BMA_WIDTH );
       		bmHeight = GetBitMapAttr( data->bm, BMA_HEIGHT );
      		InitBitMap( &maskBM, 1L, bmWidth, bmHeight );			

		rasSize = RASSIZE( bmWidth, bmHeight );

                if( ( ras = AllocVec( rasSize, MEMF_CHIP ) ) )
                {
	            if( ( data->mask = AllocVec( rasSize, MEMF_CHIP ) ) )
	            {
	            	struct AreaInfo	 ai;
	            	struct TmpRas	 tr;
	            	struct RastPort	*maskRP = &data->trp;
	            	UWORD		 buf[10] = {};

	            	maskRP->BitMap = &maskBM;
	            	maskBM.Planes[0] = data->mask;
	            	InitArea( &ai, buf, sizeof( buf ) / 5 );
	            	maskRP->AreaInfo = &ai;
	            	maskRP->TmpRas = &tr;
			InitTmpRas( &tr, ras, rasSize );

	            	SetRast( maskRP, 0L );
	            	SetAPen( maskRP, 1L );
	            	AreaEllipse( maskRP, cx,cy, rx,ry );
	            	AreaEnd( maskRP );
	            	WaitBlit();

			if (!data->savebm)
			{					
			    data->savebm = AllocBitMap( 
				    KNOBWIDTH, KNOBHEIGHT, 
				    GetBitMapAttr(rp->BitMap, BMA_DEPTH), 
				    BMF_MINPLANES, rp->BitMap );
			}

			maskRP->AreaInfo = NULL;
			maskRP->TmpRas = NULL;	
	            }

	            FreeVec( ras );
	    	}		    	
            }	

	    if (CyberGfxBase && (GetBitMapAttr(data->bm, BMA_DEPTH) >= 15))
	    {
		TrueWheel(data, &temprp, &wbox, ColorWheelBase);
	    } else {
	    	ClutWheel(data, &temprp, &wbox, ColorWheelBase);
	    }

	    SetAPen(&temprp, data->dri->dri_Pens[SHADOWPEN]);
	    DrawEllipse(&temprp, cx, cy, rx, ry);
	    DrawEllipse(&temprp, cx, cy, rx - 1, ry);
	    DrawEllipse(&temprp, cx, cy, rx, ry - 1);
	    DrawEllipse(&temprp, cx, cy, rx - 1, ry - 1);
	    
	    DeinitRastPort(&temprp);
	    	    
	} /* if (data->bm) */
		
    } /* if (!data->bm || (box->Width != data->bmwidth) || (box->Height != data->bmheight)) */
    
    if (data->bm)
    {    
        data->wheeldrawn = TRUE;

       	if(data->mask)
       	{              		
#ifdef _AROS
   		BltMaskBitMapRastPort( 
   			data->bm, 0, 0, 
  			rp, box->Left, box->Top, box->Width, box->Height, 
			0xe0, data->mask);	
#else
   		NewBltMaskBitMapRastPort( 
   			data->bm, 0, 0, 
  			rp, box->Left, box->Top, box->Width, box->Height, 
			0xe0, data->mask, (struct Library *)GfxBase, LayersBase );
#endif
       	}
       	else BltBitMapRastPort(data->bm, 0, 0, rp, box->Left, box->Top, box->Width, box->Height, 0xC0);
    }
    
}

/***************************************************************************************************/

VOID RenderKnob(struct ColorWheelData *data, struct RastPort *rp, struct IBox *gbox, BOOL update,
		struct ColorWheelBase_intern *ColorWheelBase)
{
    WORD x, y;
    
    if (!data->wheeldrawn) return;
    
    if (update)
    {
        /* Restore */
	
	if (data->savebm)	
	    BltBitMapRastPort(data->savebm,
	      0,0,
	      rp,
	      data->knobsavex + gbox->Left,
	      data->knobsavey + gbox->Top,
	      KNOBWIDTH,
	      KNOBHEIGHT,
	      0xC0);
	else
	    BltBitMapRastPort(data->bm,
	      data->knobsavex,data->knobsavey,
	      rp,
	      data->knobsavex + gbox->Left,
	      data->knobsavey + gbox->Top,
	      KNOBWIDTH,
	      KNOBHEIGHT,
	      0xC0);
    }
    
    CalcKnobPos(data, &x, &y);

    if (x < KNOBCX) x = KNOBCX; else if (x > gbox->Width  - 1 - KNOBCX) x = gbox->Width  - 1 - KNOBCX;
    if (y < KNOBCY) y = KNOBCY; else if (y > gbox->Height - 1 - KNOBCY) y = gbox->Height - 1 - KNOBCY;
    
    /* Backup */
    
    data->knobsavex = x - KNOBCX;
    data->knobsavey = y - KNOBCY;
            
    /* Render */
    
    x += gbox->Left;
    y += gbox->Top;
    
    if (data->savebm)
    {
    	data->trp.BitMap = data->savebm;
    	ClipBlit(rp, x-KNOBCX,y-KNOBCY, &data->trp, 0,0, KNOBWIDTH,KNOBHEIGHT, 0xc0 );
    }	
    
    SetDrMd(rp, JAM1);    
    
    SetAPen(rp, data->dri->dri_Pens[SHADOWPEN]);
    
    RectFill(rp, x - 3, y - 1, x - 3, y + 1);
    RectFill(rp, x - 2, y - 2, x - 2, y + 2);
    RectFill(rp, x - 1, y - 3, x + 1, y + 3);
    RectFill(rp, x + 2, y - 2, x + 2, y + 2);
    RectFill(rp, x + 3, y - 1, x + 3, y + 1);
    
    SetAPen(rp, data->dri->dri_Pens[SHINEPEN]);
    
    RectFill(rp, x - 1, y, x + 1, y);
    RectFill(rp, x, y - 1, x, y + 1);
}

/***************************************************************************************************/

VOID GetGadgetIBox(Object *o, struct GadgetInfo *gi, struct IBox *ibox)
{
    ibox->Left	 = EG(o)->LeftEdge;
    ibox->Top	 = EG(o)->TopEdge;
    ibox->Width  = EG(o)->Width;
    ibox->Height = EG(o)->Height;

    if (gi)
    {
	if (EG(o)->Flags & GFLG_RELRIGHT)
	    ibox->Left	 += gi->gi_Domain.Width - 1;

	if (EG(o)->Flags & GFLG_RELBOTTOM)
	    ibox->Top	 += gi->gi_Domain.Height - 1;

	if (EG(o)->Flags & GFLG_RELWIDTH)
	    ibox->Width  += gi->gi_Domain.Width;

	if (EG(o)->Flags & GFLG_RELHEIGHT)
	    ibox->Height += gi->gi_Domain.Height;
    }
}


/***************************************************************************************************/

void DrawDisabledPattern(struct RastPort *rport, struct IBox *gadbox, UWORD pen,
			 struct ColorWheelBase_intern *ColorWheelBase)
{
    STATIC UWORD pattern[] = { 0x8888, 0x2222 };

    EnterFunc(bug("DrawDisabledPattern(rp=%p, gadbox=%p, pen=%d)\n",
    		rport, gadbox, pen));

    SetDrMd( rport, JAM1 );
    SetAPen( rport, pen );
    SetAfPt( rport, pattern, 1);

    /* render disable pattern */
    RectFill( rport, gadbox->Left,
    		     gadbox->Top,
		     gadbox->Left + gadbox->Width  - 1,
		     gadbox->Top  + gadbox->Height - 1);
		         
    SetAfPt ( rport, NULL, 0);

    ReturnVoid("DrawDisabledPattern");
}

/***************************************************************************************************/

void allocPens( struct ColorWheelData *data, struct ColorWheelBase_intern *ColorWheelBase )
{
    if( ! ( CyberGfxBase && GetBitMapAttr( data->scr->RastPort.BitMap, BMA_DEPTH ) >= 15 ) )
    {		
	struct ColorMap *cm = data->scr->ViewPort.ColorMap;
	LONG	 	 i;

	if( data->donation )
	{		
	    for( i = 0; data->donation[i] != 0xffff; i++ );

	    data->pens_donated = i;			
	}
	else data->pens_donated = 0;

	if( ( data->pens_donated + data->maxpens ) > 0L )
	{				
	    if( ( data->pens = (UBYTE *) AllocVec( data->pens_donated + data->maxpens, MEMF_PUBLIC ) ) )
	    {
		UBYTE	*p = data->pens;

		if( data->donation )
		{
		    for( i = 0; data->donation[i] != 0xffff; i++ )
			*p++ = (UBYTE)data->donation[i];
		}				

		for( i = 0; i < data->maxpens; i++ )
		{
		    UWORD	n;

		    n = ObtainPen( cm, -1L, 0,0,0, PEN_EXCLUSIVE|PEN_NO_SETCOLOR );

		    if( n == 0xffff )
			break;

		    *p++ = n;				
		}

		data->numpens = i + data->pens_donated;

		if( data->numpens >= 8L )
		{	
		    LONG	 range, levels, r,g,b;

		    p = data->pens;

		    for( levels = 6; levels >= 2; levels-- )
		    {
			LONG	max = levels*levels*levels;

			if( max <= data->numpens )
			{
			    i = ( max < data->pens_donated ) ? data->pens_donated : max;
			    p = &data->pens[ i ];

			    for( ; i < data->numpens; i++ )
				ReleasePen( cm, *p++ );

			    data->numpens = max;

			    break;
			}	
		    }		

		    range = 255 / (levels - 1);
		    p = data->pens;

		    for(r = 0, i = 0 ; r < levels ; r++)
		    {
			for(g = 0 ; g < levels ; g++)
			{
			    for(b = 0 ; b < levels ; b++)
			    {
				ULONG	tab[] = {
					( 1 << 16 ) | ( *p++ ),
					(r*range)*0x01010101, 
					(g*range)*0x01010101,
					(b*range)*0x01010101,
					0
				};	

				LoadRGB32( &data->scr->ViewPort, tab );
			    }	
			}
		    }

		    data->range = range;
		    data->levels = levels;
		}
		else freePens( data, ColorWheelBase );
	    }
	}				
    }		

}	

/****************************************************************************/

void freePens( struct ColorWheelData *data, struct ColorWheelBase_intern *ColorWheelBase )
{
    if( data->pens )
    {
	struct ColorMap *cm = data->scr->ViewPort.ColorMap;
	LONG		 i;

	for( i = data->pens_donated; i < data->numpens; i++ )
	    ReleasePen( cm, data->pens[i] );

	FreeVec( data->pens );

	data->pens = NULL;
    }	
}	

/****************************************************************************/
