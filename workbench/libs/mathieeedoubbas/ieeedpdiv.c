/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang:
*/

#include <aros/libcall.h>
#include <proto/mathieeedoubbas.h>
#include <proto/exec.h>
#include <exec/types.h>
#include "mathieeedoubbas_intern.h"

/*****************************************************************************

    NAME */

      AROS_LHQUAD2(double, IEEEDPDiv,

/*  SYNOPSIS */
      AROS_LHAQUAD(double, y, D0, D1),
      AROS_LHAQUAD(double, z, D2, D3),

/*  LOCATION */
      struct MathIeeeDoubBasBase *, MathIeeeDoubBasBase, 14, MathIeeeDoubBas)

/*  FUNCTION
	Divides two IEEE double precision numbers

    INPUTS
	y  - IEEE double precision floating point
	z  - IEEE double precision floating point

    RESULT
       +1 : y > z
	0 : y = z
       -1 : y < z


	Flags:
	  zero	   : y = z
	  negative : y < z
	  overflow : 0

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

******************************************************************************/
{
AROS_LIBFUNC_INIT

  return 0x0badc0de0badc0deULL;

AROS_LIBFUNC_EXIT
} /* IEEEDPDiv */

