#ifndef GRAPHICS_MODEID_H
#define GRAPHICS_MODEID_H

/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc: Display mode definitions
    Lang: english
*/

#ifndef GRAPHICS_DISPLAYINFO_H
#   include <graphics/displayinfo.h>
#endif

                            /* Standard Monitors */

#define DEFAULT_MONITOR_ID 0x00000000
#define NTSC_MONITOR_ID    0x00011000
#define PAL_MONITOR_ID     0x00021000

/*
   LowResolution (LORES):   0x00000000
   Interlaced (LACE):       0x00000004 (Bit 2)
   ScanDoubled (SDBL):      0x00000008 (Bit 3)
   Super (SUPER):           0x00008020 (Bit 5 | HIRES)
   DoublePlayField2 (DPF2): 0x00000440 (Bit 6 | DPF)
   ExtraHalfBrite (EHB):    0x00000080 (Bit 7)
   DoublePlayField (DPF):   0x00000400 (Bit 10)
   Hold and Modify (HAM):   0x00000800 (Bit 11)
   HighResolution (HIRES):  0x00008000 (Bit 15)
*/
#define LORES_KEY              0x00000000
#define LORESLACE_KEY          0x00000004
#define LORESSDBL_KEY          0x00000008
#define EXTRAHALFBRITE_KEY     0x00000080
#define EXTRAHALFBRITELACE_KEY 0x00000084
#define LORESEHBSDBL_KEY       0x00000088
#define LORESDPF_KEY           0x00000400
#define LORESLACEDPF_KEY       0x00000404
#define LORESDPF2_KEY          0x00000440
#define LORESLACEDPF2_KEY      0x00000444
#define HAM_KEY                0x00000800
#define HAMLACE_KEY            0x00000804
#define LORESHAMSDBL_KEY       0x00000808
#define HIRES_KEY              0x00008000
#define HIRESLACE_KEY          0x00008004
#define SUPER_KEY              0x00008020
#define SUPERLACE_KEY          0x00008024
#define HIRESEHB_KEY           0x00008080
#define HIRESEHBLACE_KEY       0x00008084
#define SUPEREHB_KEY           0x000080A0
#define SUPEREHBLACE_KEY       0x000080A4
#define HIRESDPF_KEY           0x00008400
#define HIRESLACEDPF_KEY       0x00008404
#define SUPERDPF_KEY           0x00008420
#define SUPERLACEDPF_KEY       0x00008424
#define HIRESDPF2_KEY          0x00008440
#define HIRESLACEDPF2_KEY      0x00008444
#define SUPERDPF2_KEY          0x00008460
#define SUPERLACEDPF2_KEY      0x00008464
#define HIRESHAM_KEY           0x00008800
#define HIRESHAMLACE_KEY       0x00008804
#define HIRESHAMSDBL_KEY       0x00008808
#define SUPERHAM_KEY           0x00008820
#define SUPERHAMLACE_KEY       0x00008824

                           /* VGA Monitors */

#define VGA_MONITOR_ID 0x00031000
/*
   ExtraLowResolution (EXTRALORES): 0x00000000
   ScanDoubled (SDBL):              0x00000000
   Interlaced (LACE):               0x00000001 (Bit 0)
   Not ScanDoubled:                 0x00000004 (Bit 2)
   Productivity (PRODUCT):          0x00008020 (Bit 5 | LORES)
   DoublePlayField2 (DPF2):         0x00000440 (Bit 6 | DPF)
   HalfBrite (HB):                  0x00000080 (Bit 7)
   DoublePlayField (DPF):           0x00000400 (Bit 10)
   Hold and Modify (HAM):           0x00000800 (Bit 11)
   LowResolution (LORES):           0x00008000 (Bit 15)
*/

/* obsolete ? */
#define VGAEXTRAHALFBRITE_KEY     0x00031084
#define VGAEXTRAHALFBRITELACE_KEY 0x00031085
#define VGAHAM_KEY                0x00031804
#define VGAHAMLACE_KEY            0x00031805

#define VGAEXTRALORESDBL_KEY      0x00031000
#define VGAEXTRALORES_KEY         0x00031004
#define VGAEXTRALORESLACE_KEY     0x00031005
#define VGAEXTRALORESEHBDBL_KEY   0x00031080
#define VGAEXTRALORESHB_KEY       0x00031084
#define VGAEXTRALORESHBLACE_KEY   0x00031085
#define VGAEXTRALORESDPF_KEY      0x00031404
#define VGAEXTRALORESLACEDPF_KEY  0x00031405
#define VGAEXTRALORESDPF2_KEY     0x00031444
#define VGAEXTRALORESLACEDPF2_KEY 0x00031445
#define VGAEXTRALORESHAMDBL_KEY   0x00031800
#define VGAEXTRALORESHAM_KEY      0x00031804
#define VGAEXTRALORESHAMLACE_KEY  0x00031805
#define VGALORESDBL_KEY           0x00039000
#define VGALORES_KEY              0x00039004
#define VGALORESLACE_KEY          0x00039005
#define VGAPRODUCTDBL_KEY         0x00039020
#define VGAPRODUCT_KEY            0x00039024
#define VGAPRODUCTLACE_KEY        0x00039025
#define VGALORESEHBDBL_KEY        0x00039080
#define VGALORESHB_KEY            0x00039084
#define VGALORESHBLACE_KEY        0x00039085
#define VGAPRODUCTEHBDBL_KEY      0x000390A0
#define VGAEHB_KEY                0x000390A4
#define VGAEHBLACE_KEY            0x000390A5
#define VGALORESDPF_KEY           0x00039404
#define VGALORESLACEDPF_KEY       0x00039405
#define VGAPRODUCTDPF_KEY         0x00039424
#define VGAPRODUCTLACEDPF_KEY     0x00039425
#define VGALORESDPF2_KEY          0x00039444
#define VGALORESLACEDPF2_KEY      0x00039445
#define VGAPRODUCTDPF2_KEY        0x00039464
#define VGAPRODUCTLACEDPF2_KEY    0x00039465
#define VGALORESHAMDBL_KEY        0x00039800
#define VGALORESHAM_KEY           0x00039804
#define VGALORESHAMLACE_KEY       0x00039805
#define VGAPRODUCTHAMDBL_KEY      0x00039820
#define VGAPRODUCTHAM_KEY         0x00039824
#define VGAPRODUCTHAMLACE_KEY     0x00039825

                           /* A2024 Monitors */

#define A2024_MONITOR_ID 0x00041000

#define A2024TENHERTZ_KEY     0x00041000
#define A2024FIFTEENHERTZ_KEY 0x00049000

                          /* Euro 36 Monitors */

#define EURO36_MONITOR_ID 0x00071000

                          /* Euro 72 Monitors */

#define EURO72_MONITOR_ID 0x00061000
/*
   ExtraLowResolution (EXTRALORES): 0x00000000
   ScanDoubled (DBL):               0x00000000
   Interlaced (LACE):               0x00000001 (Bit 0)
   Not ScanDoubled:                 0x00000004 (Bit 2)
   Productivity (PRODUCT):          0x00008020 (Bit 5 | LORES)
   DoublePlayField2 (DPF2):         0x00000440 (Bit 6 | DPF)
   ExtraHalfBright (EHB):           0x00000080 (Bit 7)
   DoublePlayField (DPF):           0x00000400 (Bit 10)
   Hold and Modify (HAM):           0x00000800 (Bit 11)
   LowResolution (LORES):           0x00008000 (Bit 15)
*/

/* obsolete ? */
#define EURO72EXTRAHALFBRITE_KEY     0x00061084
#define EURO72EXTRAHALFBRITELACE_KEY 0x00061085
#define EURO72HAM_KEY                0x00061804
#define EURO72HAMLACE_KEY            0x00061805

#define EURO72EXTRALORESDBL_KEY      0x00061000
#define EURO72EXTRALORES_KEY         0x00061004
#define EURO72EXTRALORESLACE_KEY     0x00061005
#define EURO72EXTRALORESEHBDBL_KEY   0x00061080
#define EURO72EXTRALORESEHB_KEY      0x00061084
#define EURO72EXTRALORESEHBLACE_KEY  0x00061085
#define EURO72EXTRALORESDPF_KEY      0x00061404
#define EURO72EXTRALORESLACEDPF_KEY  0x00061405
#define EURO72EXTRALORESDPF2_KEY     0x00061444
#define EURO72EXTRALORESLACEDPF2_KEY 0x00061445
#define EURO72EXTRALORESHAMDBL_KEY   0x00061800
#define EURO72EXTRALORESHAM_KEY      0x00061804
#define EURO72EXTRALORESHAMLACE_KEY  0x00061805
#define EURO71LORESDBL_KEY           0x00069000
#define EURO72LORES_KEY              0x00069004
#define EURO72LORESLACE_KEY          0x00069005
#define EURO72PRODUCTDBL_KEY         0x00069020
#define EURO72PRODUCT_KEY            0x00069024
#define EURO72PRODUCTLACE_KEY        0x00069025
#define EURO72LORESEHBDBL_KEY        0x00069080
#define EURO72LORESEHB_KEY           0x00069084
#define EURO72LORESEHBLACE_KEY       0x00069085
#define EURO72PRODUCTEHBDBL_KEY      0x000690A0
#define EURO72EHB_KEY                0x000690A4
#define EURO72EHBLACE_KEY            0x000690A5
#define EURO72LORESDPF_KEY           0x00069404
#define EURO72LORESLACEDPF_KEY       0x00069405
#define EURO72PRODUCTDPF_KEY         0x00069424
#define EURO72PRODUCTLACEDPF_KEY     0x00069425
#define EURO72LORESDPF2_KEY          0x00069444
#define EURO72LORESLACEDPF2_KEY      0x00069445
#define EURO72PRODUCTDPF2_KEY        0x00069464
#define EURO72PRODUCTLACEDPF2_KEY    0x00069465
#define EURO72LORESHAMDBL_KEY        0x00069800
#define EURO72LORESHAM_KEY           0x00069804
#define EURO72LORESHAMLACE_KEY       0x00069805
#define EURO72PRODUCTHAMDBL_KEY      0x00069820
#define EURO72PRODUCTHAM_KEY         0x00069824
#define EURO72PRODUCTHAMLACE_KEY     0x00069825

                          /* Super72 Monitors */

#define SUPER72_MONITOR_ID 0x00081000

#define SUPER72LORESDBL_KEY    0x00081008
#define SUPER72LORESEHBDBL_KEY 0x00081088
#define SUPER72LORESHAMDBL_KEY 0x00081808
#define SUPER72HIRESDBL_KEY    0x00089008
#define SUPER72SUPERDBL_KEY    0x00089028
#define SUPER72HIRESEHBDBL_KEY 0x00089088
#define SUPER72SUPEREHBDBL_KEY 0x000890A8
#define SUPER72HIRESHAMDBL_KEY 0x00089808
#define SUPER72SUPERHAMDBL_KEY 0x00089828

                          /* DBLNTSC Monitors */

#define DBLNTSC_MONITOR_ID 0x00091000

#define DBLNTSCLORES_KEY              0x00091000
#define DBLNTSCLORESFF_KEY            0x00091004
#define DBLNTSCLORESLACE_KEY          0x00091005
#define DBLNTSCLORESEHB_KEY           0x00091080
#define DBLNTSCLORESEHBFF_KEY         0x00091084
#define DBLNTSCLORESEHBLACE_KEY       0x00091085
#define DBLNTSCEXTRALORES_KEY         0x00091200
#define DBLNTSCEXTRALORESFF_KEY       0x00091204
#define DBLNTSCEXTRALORESLACE_KEY     0x00091205
#define DBLNTSCEXTRALORESEHB_KEY      0x00091280
#define DBLNTSCEXTRALORESEHBFF_KEY    0x00091284
#define DBLNTSCEXTRALORESEHBLACE_KEY  0x00091285
#define DBLNTSCLORESDPF_KEY           0x00091400
#define DBLNTSCLORESDPFFF_KEY         0x00091404
#define DBLNTSCLORESDPFLACE_KEY       0x00091405
#define DBLNTSCLORESDPF2_KEY          0x00091440
#define DBLNTSCLORESDPF2FF_KEY        0x00091444
#define DBLNTSCLORESDPF2LACE_KEY      0x00091445
#define DBLNTSCEXTRALORESDPF_KEY      0x00091600
#define DBLNTSCEXTRALORESDPFFF_KEY    0x00091604
#define DBLNTSCEXTRALORESDPFLACE_KEY  0x00091605
#define DBLNTSCEXTRALORESDPF2_KEY     0x00091640
#define DBLNTSCEXTRALORESDPF2FF_KEY   0x00091644
#define DBLNTSCEXTRALORESDPF2LACE_KEY 0x00091645
#define DBLNTSCLORESHAM_KEY           0x00091800
#define DBLNTSCLORESHAMFF_KEY         0x00091804
#define DBLNTSCLORESHAMLACE_KEY       0x00091805
#define DBLNTSCEXTRALORESHAM_KEY      0x00091A00
#define DBLNTSCEXTRALORESHAMFF_KEY    0x00091A04
#define DBLNTSCEXTRALORESHAMLACE_KEY  0x00091A05
#define DBLNTSCHIRES_KEY              0x00099000
#define DBLNTSCHIRESFF_KEY            0x00099004
#define DBLNTSCHIRESLACE_KEY          0x00099005
#define DBLNTSCHIRESEHB_KEY           0x00099080
#define DBLNTSCHIRESEHBFF_KEY         0x00099084
#define DBLNTSCHIRESEHBLACE_KEY       0x00099085
#define DBLNTSCHIRESDPF_KEY           0x00099400
#define DBLNTSCHIRESDPFFF_KEY         0x00099404
#define DBLNTSCHIRESDPFLACE_KEY       0x00099405
#define DBLNTSCHIRESDPF2_KEY          0x00099440
#define DBLNTSCHIRESDPF2FF_KEY        0x00099444
#define DBLNTSCHIRESDPF2LACE_KEY      0x00099445
#define DBLNTSCHIRESHAM_KEY           0x00099800
#define DBLNTSCHIRESHAMFF_KEY         0x00099804
#define DBLNTSCHIRESHAMLACE_KEY       0x00099805

                          /* DBLPAL Monitors */

#define DBLPAL_MONITOR_ID 0x000A1000

#define DBLPALLORES_KEY              0x000A1000
#define DBLPALLORESFF_KEY            0x000A1004
#define DBLPALLORESLACE_KEY          0x000A1005
#define DBLPALLORESEHB_KEY           0x000A1080
#define DBLPALLORESEHBFF_KEY         0x000A1084
#define DBLPALLORESEHBLACE_KEY       0x000A1085
#define DBLPALEXTRALORES_KEY         0x000A1200
#define DBLPALEXTRALORESFF_KEY       0x000A1204
#define DBLPALEXTRALORESLACE_KEY     0x000A1205
#define DBLPALEXTRALORESEHB_KEY      0x000A1280
#define DBLPALEXTRALORESEHBFF_KEY    0x000A1284
#define DBLPALEXTRALORESEHBLACE_KEY  0x000A1285
#define DBLPALLORESDPF_KEY           0x000A1400
#define DBLPALLORESDPFFF_KEY         0x000A1404
#define DBLPALLORESDPFLACE_KEY       0x000A1405
#define DBLPALLORESDPF2_KEY          0x000A1440
#define DBLPALLORESDPF2FF_KEY        0x000A1444
#define DBLPALLORESDPF2LACE_KEY      0x000A1445
#define DBLPALEXTRALORESDPF_KEY      0x000A1600
#define DBLPALEXTRALORESDPFFF_KEY    0x000A1604
#define DBLPALEXTRALORESDPFLACE_KEY  0x000A1605
#define DBLPALEXTRALORESDPF2_KEY     0x000A1640
#define DBLPALEXTRALORESDPF2FF_KEY   0x000A1644
#define DBLPALEXTRALORESDPF2LACE_KEY 0x000A1645
#define DBLPALLORESHAM_KEY           0x000A1800
#define DBLPALLORESHAMFF_KEY         0x000A1804
#define DBLPALLORESHAMLACE_KEY       0x000A1805
#define DBLPALEXTRALORESHAM_KEY      0x000A1A00
#define DBLPALEXTRALORESHAMFF_KEY    0x000A1A04
#define DBLPALEXTRALORESHAMLACE_KEY  0x000A1A05
#define DBLPALHIRES_KEY              0x000A9000
#define DBLPALHIRESFF_KEY            0x000A9004
#define DBLPALHIRESLACE_KEY          0x000A9005
#define DBLPALHIRESEHB_KEY           0x000A9080
#define DBLPALHIRESEHBFF_KEY         0x000A9084
#define DBLPALHIRESEHBLACE_KEY       0x000A9085
#define DBLPALHIRESDPF_KEY           0x000A9400
#define DBLPALHIRESDPFFF_KEY         0x000A9404
#define DBLPALHIRESDPFLACE_KEY       0x000A9405
#define DBLPALHIRESDPF2_KEY          0x000A9440
#define DBLPALHIRESDPF2FF_KEY        0x000A9444
#define DBLPALHIRESDPF2LACE_KEY      0x000A9445
#define DBLPALHIRESHAM_KEY           0x000A9800
#define DBLPALHIRESHAMFF_KEY         0x000A9804
#define DBLPALHIRESHAMLACE_KEY       0x000A9805

                           /* Miscellaneous */

/* PRIVATE */
#define PROTO_MONITOR_ID 0x00051000

#define INVALID_ID ~0

#define MONITOR_ID_MASK 0xFFFF1000

                           /* BestModeID() */

/* Tags */
#define BIDTAG_DIPFMustHave    0x80000001
#define BIDTAG_DIPFMustNotHave 0x80000002
#define BIDTAG_ViewPort        0x80000003
#define BIDTAG_NominalWidth    0x80000004
#define BIDTAG_NominalHeight   0x80000005
#define BIDTAG_DesiredWidth    0x80000006
#define BIDTAG_DesiredHeight   0x80000007
#define BIDTAG_Depth           0x80000008
#define BIDTAG_MonitorID       0x80000009
#define BIDTAG_SourceID        0x8000000A
#define BIDTAG_RedBits         0x8000000B
#define BIDTAG_BlueBits        0x8000000C
#define BIDTAG_GreenBits       0x8000000D

#endif /* GRAPHICS_MODEID_H */
