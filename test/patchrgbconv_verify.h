UWORD testpixels_RGB15[] =
{
    DOWNSHIFT16(0, ARGB32, RGB15),
    DOWNSHIFT16(0xFFFFFFFF, ARGB32, RGB15),
    DOWNSHIFT16(0xFF000000, ARGB32, RGB15),
    DOWNSHIFT16(0xFF00FF00, ARGB32, RGB15),
    DOWNSHIFT16(0x00FF00FF, ARGB32, RGB15),
    DOWNSHIFT16(0x00123456, ARGB32, RGB15),
    DOWNSHIFT16(0x00FFFF00, ARGB32, RGB15),
    DOWNSHIFT16(0x778899AA, ARGB32, RGB15),    
};

const int NUMTESTPIXELS_RGB15 = ( sizeof(testpixels_RGB15) / sizeof(testpixels_RGB15[0]) );

UWORD testpixels_BGR15[] =
{
    DOWNSHIFT16(0, ARGB32, BGR15),
    DOWNSHIFT16(0xFFFFFFFF, ARGB32, BGR15),
    DOWNSHIFT16(0xFF000000, ARGB32, BGR15),
    DOWNSHIFT16(0xFF00FF00, ARGB32, BGR15),
    DOWNSHIFT16(0x00FF00FF, ARGB32, BGR15),
    DOWNSHIFT16(0x00123456, ARGB32, BGR15),
    DOWNSHIFT16(0x00FFFF00, ARGB32, BGR15),
    DOWNSHIFT16(0x778899AA, ARGB32, BGR15),    
};

const int NUMTESTPIXELS_BGR15 = ( sizeof(testpixels_BGR15) / sizeof(testpixels_BGR15[0]) );

UWORD testpixels_RGB16[] =
{
    DOWNSHIFT16(0, ARGB32, RGB16),
    DOWNSHIFT16(0xFFFFFFFF, ARGB32, RGB16),
    DOWNSHIFT16(0xFF000000, ARGB32, RGB16),
    DOWNSHIFT16(0xFF00FF00, ARGB32, RGB16),
    DOWNSHIFT16(0x00FF00FF, ARGB32, RGB16),
    DOWNSHIFT16(0x00123456, ARGB32, RGB16),
    DOWNSHIFT16(0x00FFFF00, ARGB32, RGB16),
    DOWNSHIFT16(0x778899AA, ARGB32, RGB16),    
};

const int NUMTESTPIXELS_RGB16 = ( sizeof(testpixels_RGB16) / sizeof(testpixels_RGB16[0]) );

UWORD testpixels_BGR16[] =
{
    DOWNSHIFT16(0, ARGB32, BGR16),
    DOWNSHIFT16(0xFFFFFFFF, ARGB32, BGR16),
    DOWNSHIFT16(0xFF000000, ARGB32, BGR16),
    DOWNSHIFT16(0xFF00FF00, ARGB32, BGR16),
    DOWNSHIFT16(0x00FF00FF, ARGB32, BGR16),
    DOWNSHIFT16(0x00123456, ARGB32, BGR16),
    DOWNSHIFT16(0x00FFFF00, ARGB32, BGR16),
    DOWNSHIFT16(0x778899AA, ARGB32, BGR16),    
};

const int NUMTESTPIXELS_BGR16 = ( sizeof(testpixels_BGR16) / sizeof(testpixels_BGR16[0]) );

ULONG testpixels_ARGB32[] =
{
    SHUFFLE32(0, ARGB32, ARGB32),
    SHUFFLE32(0xFFFFFFFF, ARGB32, ARGB32),
    SHUFFLE32(0xFF000000, ARGB32, ARGB32),
    SHUFFLE32(0xFF00FF00, ARGB32, ARGB32),
    SHUFFLE32(0x00FF00FF, ARGB32, ARGB32),
    SHUFFLE32(0x00123456, ARGB32, ARGB32),
    SHUFFLE32(0x00FFFF00, ARGB32, ARGB32),
    SHUFFLE32(0x778899AA, ARGB32, ARGB32),    
};

const int NUMTESTPIXELS_ARGB32 = ( sizeof(testpixels_ARGB32) / sizeof(testpixels_ARGB32[0]) );

ULONG testpixels_BGRA32[] =
{
    SHUFFLE32(0, ARGB32, BGRA32),
    SHUFFLE32(0xFFFFFFFF, ARGB32, BGRA32),
    SHUFFLE32(0xFF000000, ARGB32, BGRA32),
    SHUFFLE32(0xFF00FF00, ARGB32, BGRA32),
    SHUFFLE32(0x00FF00FF, ARGB32, BGRA32),
    SHUFFLE32(0x00123456, ARGB32, BGRA32),
    SHUFFLE32(0x00FFFF00, ARGB32, BGRA32),
    SHUFFLE32(0x778899AA, ARGB32, BGRA32),    
};

const int NUMTESTPIXELS_BGRA32 = ( sizeof(testpixels_BGRA32) / sizeof(testpixels_BGRA32[0]) );

ULONG testpixels_RGBA32[] =
{
    SHUFFLE32(0, ARGB32, RGBA32),
    SHUFFLE32(0xFFFFFFFF, ARGB32, RGBA32),
    SHUFFLE32(0xFF000000, ARGB32, RGBA32),
    SHUFFLE32(0xFF00FF00, ARGB32, RGBA32),
    SHUFFLE32(0x00FF00FF, ARGB32, RGBA32),
    SHUFFLE32(0x00123456, ARGB32, RGBA32),
    SHUFFLE32(0x00FFFF00, ARGB32, RGBA32),
    SHUFFLE32(0x778899AA, ARGB32, RGBA32),    
};

const int NUMTESTPIXELS_RGBA32 = ( sizeof(testpixels_RGBA32) / sizeof(testpixels_RGBA32[0]) );

ULONG testpixels_ABGR32[] =
{
    SHUFFLE32(0, ARGB32, ABGR32),
    SHUFFLE32(0xFFFFFFFF, ARGB32, ABGR32),
    SHUFFLE32(0xFF000000, ARGB32, ABGR32),
    SHUFFLE32(0xFF00FF00, ARGB32, ABGR32),
    SHUFFLE32(0x00FF00FF, ARGB32, ABGR32),
    SHUFFLE32(0x00123456, ARGB32, ABGR32),
    SHUFFLE32(0x00FFFF00, ARGB32, ABGR32),
    SHUFFLE32(0x778899AA, ARGB32, ABGR32),    
};

const int NUMTESTPIXELS_ABGR32 = ( sizeof(testpixels_ABGR32) / sizeof(testpixels_ABGR32[0]) );

UBYTE testpixels_RGB24[] =
{
     0, 0, 0,
     0xFF, 0xFF, 0xFF,
     0xFF, 0, 0,
     0xFF, 0xFF, 0,
     0xFF, 0x0, 0xFF,
     0x12, 0x34, 0x56,
     0x00, 0xFF, 0x77,
     0x88, 0x99, 0xAA,
};

const int NUMTESTPIXELS_RGB24 = ( sizeof(testpixels_RGB24) / sizeof(testpixels_RGB24[0]) / 3);

#undef ACTFMT
#define ACTFMT BGR24

UBYTE testpixels_BGR24[] =
{
     0, 0, 0,
     0xFF, 0xFF, 0xFF,
     0xFF, 0, 0,
     0xFF, 0xFF, 0,
     0xFF, 0x0, 0xFF,
     0x12, 0x34, 0x56,
     0x00, 0xFF, 0x77,
     0x88, 0x99, 0xAA,
};

const int NUMTESTPIXELS_BGR24 = ( sizeof(testpixels_BGR24) / sizeof(testpixels_BGR24[0]) / 3);

#define SW(x) ( (((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8) )

UWORD testpixels_RGB16OE[] =
{
    SW(DOWNSHIFT16(0, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0xFFFFFFFF, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0xFF000000, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0xFF00FF00, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0x00FF00FF, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0x00123456, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0x00FFFF00, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0x778899AA, ARGB32, RGB16)),    
};

const int NUMTESTPIXELS_RGB16OE = ( sizeof(testpixels_RGB16OE) / sizeof(testpixels_RGB16OE[0]) );

UWORD testpixels_BGR16OE[] =
{
    SW(DOWNSHIFT16(0, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0xFFFFFFFF, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0xFF000000, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0xFF00FF00, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0x00FF00FF, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0x00123456, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0x00FFFF00, ARGB32, RGB16)),
    SW(DOWNSHIFT16(0x778899AA, ARGB32, RGB16)),    
};

const int NUMTESTPIXELS_BGR16OE = ( sizeof(testpixels_BGR16OE) / sizeof(testpixels_BGR16OE[0]) );

UWORD testpixels_RGB15OE[] =
{
    SW(DOWNSHIFT16(0, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0xFFFFFFFF, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0xFF000000, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0xFF00FF00, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0x00FF00FF, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0x00123456, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0x00FFFF00, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0x778899AA, ARGB32, RGB15)),    
};

const int NUMTESTPIXELS_RGB15OE = ( sizeof(testpixels_RGB15OE) / sizeof(testpixels_RGB15OE[0]) );

UWORD testpixels_BGR15OE[] =
{
    SW(DOWNSHIFT16(0, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0xFFFFFFFF, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0xFF000000, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0xFF00FF00, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0x00FF00FF, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0x00123456, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0x00FFFF00, ARGB32, RGB15)),
    SW(DOWNSHIFT16(0x778899AA, ARGB32, RGB15)),    
};

const int NUMTESTPIXELS_BGR15OE = ( sizeof(testpixels_BGR15OE) / sizeof(testpixels_BGR15OE[0]) );

ULONG testpixels_XRGB32[] =
{
    SHUFFLE24(0, ARGB32, ARGB32),
    SHUFFLE24(0xFFFFFFFF, ARGB32, ARGB32),
    SHUFFLE24(0xFF000000, ARGB32, ARGB32),
    SHUFFLE24(0xFF00FF00, ARGB32, ARGB32),
    SHUFFLE24(0x00FF00FF, ARGB32, ARGB32),
    SHUFFLE24(0x00123456, ARGB32, ARGB32),
    SHUFFLE24(0x00FFFF00, ARGB32, ARGB32),
    SHUFFLE24(0x778899AA, ARGB32, ARGB32),    
};

const int NUMTESTPIXELS_XRGB32 = ( sizeof(testpixels_XRGB32) / sizeof(testpixels_XRGB32[0]) );

ULONG testpixels_BGRX32[] =
{
    SHUFFLE24(0, ARGB32, BGRA32),
    SHUFFLE24(0xFFFFFFFF, ARGB32, BGRA32),
    SHUFFLE24(0xFF000000, ARGB32, BGRA32),
    SHUFFLE24(0xFF00FF00, ARGB32, BGRA32),
    SHUFFLE24(0x00FF00FF, ARGB32, BGRA32),
    SHUFFLE24(0x00123456, ARGB32, BGRA32),
    SHUFFLE24(0x00FFFF00, ARGB32, BGRA32),
    SHUFFLE24(0x778899AA, ARGB32, BGRA32),    
};

const int NUMTESTPIXELS_BGRX32 = ( sizeof(testpixels_BGRX32) / sizeof(testpixels_BGRX32[0]) );

ULONG testpixels_RGBX32[] =
{
    SHUFFLE24(0, ARGB32, RGBA32),
    SHUFFLE24(0xFFFFFFFF, ARGB32, RGBA32),
    SHUFFLE24(0xFF000000, ARGB32, RGBA32),
    SHUFFLE24(0xFF00FF00, ARGB32, RGBA32),
    SHUFFLE24(0x00FF00FF, ARGB32, RGBA32),
    SHUFFLE24(0x00123456, ARGB32, RGBA32),
    SHUFFLE24(0x00FFFF00, ARGB32, RGBA32),
    SHUFFLE24(0x778899AA, ARGB32, RGBA32),    
};

const int NUMTESTPIXELS_RGBX32 = ( sizeof(testpixels_RGBX32) / sizeof(testpixels_RGBX32[0]) );

ULONG testpixels_XBGR32[] =
{
    SHUFFLE24(0, ARGB32, ABGR32),
    SHUFFLE24(0xFFFFFFFF, ARGB32, ABGR32),
    SHUFFLE24(0xFF000000, ARGB32, ABGR32),
    SHUFFLE24(0xFF00FF00, ARGB32, ABGR32),
    SHUFFLE24(0x00FF00FF, ARGB32, ABGR32),
    SHUFFLE24(0x00123456, ARGB32, ABGR32),
    SHUFFLE24(0x00FFFF00, ARGB32, ABGR32),
    SHUFFLE24(0x778899AA, ARGB32, ABGR32),    
};

const int NUMTESTPIXELS_XBGR32 = ( sizeof(testpixels_XBGR32) / sizeof(testpixels_XBGR32[0]) );


#undef SW