/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: Query the current time and/or timezone
    Lang: english
*/

#include <dos/dos.h>
#include <proto/dos.h>

long __gmtoffset;

/*****************************************************************************

    NAME */
#include <sys/time.h>
#include <unistd.h>

	int gettimeofday (

/*  SYNOPSIS */
	struct timeval	* tv,
	struct timezone * tz)

/*  FUNCTION
	Return the current time and/or timezone.

    INPUTS
	tv - If this pointer is non-NULL, the current time will be
		stored here. The structure looks like this:

		struct timeval
		{
		    long tv_sec;	// seconds
		    long tv_usec;	// microseconds
		};

	tz - If this pointer is non-NULL, the current timezone will be
		stored here. The structure looks like this:

		struct timezone
		{
		    int  tz_minuteswest; // minutes west of Greenwich
		    int  tz_dsttime;	 // type of dst correction
		};

		With daylight savings times defined as follows :

		DST_NONE	// not on dst
		DST_USA 	// USA style dst
		DST_AUST	// Australian style dst
		DST_WET 	// Western European dst
		DST_MET 	// Middle European dst
		DST_EET 	// Eastern European dst
		DST_CAN 	// Canada
		DST_GB		// Great Britain and Eire
		DST_RUM 	// Rumania
		DST_TUR 	// Turkey
		DST_AUSTALT	// Australian style with shift in 1986

		And the following macros are defined to operate on this :

		timerisset(tv) - TRUE if tv contains a time

		timercmp(tv1, tv2, cmp) - Return the result of the
			comparison "tv1 cmp tv2"

		timerclear(tv) - Clear the timeval struct

    RESULT
	The number of seconds.

    NOTES

    EXAMPLE
	struct timeval tv;

	// Get the current time and print it
	gettimeofday (&tv, NULL);

	printf ("Seconds = %ld, uSec = %ld\n", tv->tv_sec, tv->tv_usec);

    BUGS

    SEE ALSO
	ctime(), asctime(), localtime(), time()

    INTERNALS

    HISTORY
	29.01.1997 digulla created

******************************************************************************/
{
    struct DateStamp t;

    DateStamp (&t); /* Get timestamp */

    /*
	2922 is the number of days between 1.1.1970 and 1.1.1978 (2 leap
		years and 6 normal). The former number is the start value
		for time(), the latter the start time for the AmigaOS
		time functions.
	1440 is the number of minutes per day
	60 is the number of seconds per minute
    */
    if (tv)
    {
	tv->tv_sec = ((t.ds_Days + 2922) * 1440 + t.ds_Minute + __gmtoffset) * 60
		+ t.ds_Tick / TICKS_PER_SECOND;
	tv->tv_usec = (t.ds_Tick % TICKS_PER_SECOND)
		* (1000000 / TICKS_PER_SECOND);
    }

    if (tz)
    {
	tz->tz_minuteswest = __gmtoffset;
#warning TODO: set tz->tz_dsttime
	tz->tz_dsttime	   = DST_NONE;
    }

    return 0;
} /* gettimeofday */

