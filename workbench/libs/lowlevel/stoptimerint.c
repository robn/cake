/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc: 
    Lang: english
*/
#include "lowlevel_intern.h"

#include <aros/libcall.h>
#include <exec/types.h>
#include <libraries/lowlevel.h>

/*****************************************************************************

    NAME */

      AROS_LH1(VOID, StopTimerInt,

/*  SYNOPSIS */ 
      AROS_LHA(APTR , intHandle, A1),

/*  LOCATION */
      struct LowLevelBase *, LowLevelBase, 20, LowLevel)

/*  NAME
 
    FUNCTION
 
    INPUTS
 
    RESULT
 
    BUGS

    INTERNALS

    HISTORY

*****************************************************************************/
{
  AROS_LIBFUNC_INIT

#warning TODO: Write lowlevel/StopTimerInt()
    aros_print_not_implemented ("lowlevel/StopTimerInt");

    return;

  AROS_LIBFUNC_EXIT
} /* StopTimerInt */
