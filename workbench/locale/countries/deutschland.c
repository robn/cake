/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Country data for Germany/Deutschland
    Lang:
*/

#include <exec/types.h>
#include <libraries/locale.h>
#include <libraries/iffparse.h>
#include <prefs/locale.h>

/* germany.country: Sebastian Rittau, srittau@jroger.in-berlin.de */
struct CountryPrefs deutschPrefs =
{
    /* Reserved */
    { 0, 0, 0, 0 },

    /* The country codes in the past have been rather inconsistant,
       sometimes they are 1 character, 2 chars or 3. It would be nice
       to have some consistency. Maybe use the 3 character name from
       ISO 3166? I (Iain) have a copy of the ISO3166 codes if anyone
       wants them...
    */

    /* Country code (left justify), telephone code, measuring system */
    MAKE_ID('D',0,0,0), 49, MS_ISO,

    /* Date time format, date format, time format */
    "%A, %e. %B %Y %H:%M:%S",
    "%A, %e. %B %Y",
    "%H:%M:%S",

    /* Short datetime, short date, short time formats */
    "%d.%m.%y %H:%M:%S",
    "%d.%m.%y",
    "%H:%M:%S",

    /* Decimal point, group separator, frac group separator */
    ",", ".", ".",

    /* For grouping rules, see <libraries/locale.h> */

    /* Grouping, Frac Grouping */
    { 3 }, { 3 },

    /* Mon dec pt, mon group sep, mon frac group sep */
    ",", ".", ".",

    /* Mon Grouping, Mon frac grouping */
    { 3 }, { 3 },

    /* Mon Frac digits, Mon IntFrac digits, then number of digits in
       the fractional part of the money value. Most countries that
       use dollars and cents, would have 2 for this value

       (As would many of those who don't).
    */
    2, 2,

#ifdef _EURO
    /* Currency symbol, Small currency symbol */
    "Euro", "Cent",

    /* Int CS, this is the ISO 4217 symbol, followed by the character to
       separate that symbol from the rest of the money. (\x00 for none).
    */
    "EUR",
#else
    /* Currency symbol, Small currency symbol */
    "DM", "Pf",

    /* Int CS, this is the ISO 4217 symbol, followed by the character to
       separate that symbol from the rest of the money. (\x00 for none).
    */
    "DM",
#endif
    /* Mon +ve sign, +ve space sep, +ve sign pos, +ve cs pos */
    "", SS_SPACE, SP_PREC_ALL, CSP_SUCCEEDS,

    /* Mon -ve sign, -ve space sep, -ve sign pos, -ve cs pos */
    "-", SS_SPACE, SP_PREC_ALL, CSP_SUCCEEDS,

    /* Calendar type */
    CT_7MON
};
