/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: Amiga bootloader -- quick SAS/C compat register declarations for GCC
    Lang: english
*/

#ifdef __GNUC__
#define __d0 __asm("d0")
#define __d1 __asm("d1")
#define __d2 __asm("d2")
#define __d3 __asm("d3")
#define __d4 __asm("d4")
#define __d5 __asm("d5")
#define __d6 __asm("d6")
#define __d7 __asm("d7")

#define __a0 __asm("a0")
#define __a1 __asm("a1")
#define __a2 __asm("a2")
#define __a3 __asm("a3")
#define __a4 __asm("a4")
#define __a5 __asm("a5")
#define __a6 __asm("a6")
#endif
