/*
    Copyright (C) 1995-1998 AROS - The Amiga Replacement OS
    $Id$

    Desc: IsXXXX() - Stub for Language isXXXXX() functions.
    Lang: english
*/
#include "locale_intern.h"
#include <aros/asmcall.h>
#include <proto/locale.h>

/*****************************************************************************

    NAME
#include <proto/locale.h>

	AROS_LH2(BOOL, IsXXXX,

    SYNOPSIS
	AROS_LHA(struct Locale *,   locale, A0),
	AROS_LHA(ULONG,             character, D0),

    LOCATION
	struct LocaleBase *, LocaleBase, 0, Locale)

    FUNCTION
	These functions allow you to find out whether a character
	matches a certain type according to the current Locale
	settings.

	The functions available are:

	IsAlNum()  - is this an alphanumeric character
	IsAlpha()  - is this an alphabet character
	IsCntrl()  - is this a control character
	IsDigit()  - is this a decimal digit character
	IsGraph()  - is this a graphical character
	IsLower()  - is this a lowercase character
	IsPrint()  - is this a printable character
	IsPunct()  - is this a punctuation character
	IsSpace()  - is this a whitespace character
	IsUpper()  - is this an uppercase character
	IsXDigit() - is this a hexadecimal digit

    INPUTS
	locale      - The Locale to use for this function.
	character   - the character to test

    RESULT
	ind - An indication of whether the character matches the type.
	    TRUE - if the character is of the required type,
	    FALSE - otherwise

    NOTES
	The Locale MUST be supplied.

	These functions require a 32-bit character to support future
	multi-byte character sets.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	27-11-96    digulla automatically created from
			    locale_lib.fd and clib/locale_protos.h

*****************************************************************************/


AROS_LH2(BOOL, IsAlNum,
    AROS_LHA(struct Locale *, locale, A0),
    AROS_LHA(ULONG          , character, D0),
    struct LocaleBase *, LocaleBase, 14, Locale)
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct LocaleBase *,LocaleBase)

    return AROS_UFC2(BOOL, locale->LanguageFunction[4],
	AROS_UFCA(ULONG,    character, D0),
	AROS_UFCA(struct LocaleBase *, LocaleBase, A6));

    AROS_LIBFUNC_EXIT
} /* IsAlNum */

AROS_LH2(BOOL, IsAlpha,
    AROS_LHA(struct Locale *, locale, A0),
    AROS_LHA(ULONG          , character, D0),
    struct LocaleBase *, LocaleBase, 15, Locale)
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct LocaleBase *,LocaleBase)

    return AROS_UFC2(BOOL, locale->LanguageFunction[5],
	AROS_UFCA(ULONG,    character, D0),
	AROS_UFCA(struct LocaleBase *, LocaleBase, A6));

    AROS_LIBFUNC_EXIT
} /* IsAlpha */

AROS_LH2(BOOL, IsCntrl,
    AROS_LHA(struct Locale *, locale, A0),
    AROS_LHA(ULONG          , character, D0),
    struct LocaleBase *, LocaleBase, 16, Locale)
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct LocaleBase *,LocaleBase)

    return AROS_UFC2(BOOL, locale->LanguageFunction[6],
	AROS_UFCA(ULONG,    character, D0),
	AROS_UFCA(struct LocaleBase *, LocaleBase, A6));

    AROS_LIBFUNC_EXIT
} /* IsCntrl */

AROS_LH2(BOOL, IsDigit,
    AROS_LHA(struct Locale *, locale, A0),
    AROS_LHA(ULONG          , character, D0),
    struct LocaleBase *, LocaleBase, 17, Locale)
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct LocaleBase *,LocaleBase)

    return AROS_UFC2(BOOL, locale->LanguageFunction[7],
	AROS_UFCA(ULONG,    character, D0),
	AROS_UFCA(struct LocaleBase *, LocaleBase, A6));

    AROS_LIBFUNC_EXIT
} /* IsDigit */

AROS_LH2(BOOL, IsGraph,
    AROS_LHA(struct Locale *, locale, A0),
    AROS_LHA(ULONG          , character, D0),
    struct LocaleBase *, LocaleBase, 18, Locale)
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct LocaleBase *,LocaleBase)

    return AROS_UFC2(BOOL, locale->LanguageFunction[8],
	AROS_UFCA(ULONG,    character, D0),
	AROS_UFCA(struct LocaleBase *, LocaleBase, A6));

    AROS_LIBFUNC_EXIT
} /* IsGraph */

AROS_LH2(BOOL, IsLower,
    AROS_LHA(struct Locale *, locale, A0),
    AROS_LHA(ULONG          , character, D0),
    struct LocaleBase *, LocaleBase, 19, Locale)
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct LocaleBase *,LocaleBase)

    return AROS_UFC2(BOOL, locale->LanguageFunction[9],
	AROS_UFCA(ULONG,    character, D0),
	AROS_UFCA(struct LocaleBase *, LocaleBase, A6));

    AROS_LIBFUNC_EXIT
} /* IsLower */

AROS_LH2(BOOL, IsPrint,
    AROS_LHA(struct Locale *, locale, A0),
    AROS_LHA(ULONG          , character, D0),
    struct LocaleBase *, LocaleBase, 20, Locale)
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct LocaleBase *,LocaleBase)

    return AROS_UFC2(BOOL, locale->LanguageFunction[10],
	AROS_UFCA(ULONG,    character, D0),
	AROS_UFCA(struct LocaleBase *, LocaleBase, A6));

    AROS_LIBFUNC_EXIT
} /* IsPrint */

AROS_LH2(BOOL, IsPunct,
    AROS_LHA(struct Locale *, locale, A0),
    AROS_LHA(ULONG          , character, D0),
    struct LocaleBase *, LocaleBase, 21, Locale)
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct LocaleBase *,LocaleBase)

    return AROS_UFC2(BOOL, locale->LanguageFunction[11],
	AROS_UFCA(ULONG,    character, D0),
	AROS_UFCA(struct LocaleBase *, LocaleBase, A6));

    AROS_LIBFUNC_EXIT
} /* IsPunct */

AROS_LH2(BOOL, IsSpace,
    AROS_LHA(struct Locale *, locale, A0),
    AROS_LHA(ULONG          , character, D0),
    struct LocaleBase *, LocaleBase, 22, Locale)
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct LocaleBase *,LocaleBase)

    return AROS_UFC2(BOOL, locale->LanguageFunction[12],
	AROS_UFCA(ULONG,    character, D0),
	AROS_UFCA(struct LocaleBase *, LocaleBase, A6));

    AROS_LIBFUNC_EXIT
} /* IsSpace */

AROS_LH2(BOOL, IsUpper,
    AROS_LHA(struct Locale *, locale, A0),
    AROS_LHA(ULONG          , character, D0),
    struct LocaleBase *, LocaleBase, 23, Locale)
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct LocaleBase *,LocaleBase)

    return AROS_UFC2(BOOL, locale->LanguageFunction[13],
	AROS_UFCA(ULONG,    character, D0),
	AROS_UFCA(struct LocaleBase *, LocaleBase, A6));

    AROS_LIBFUNC_EXIT
} /* IsUpper */

AROS_LH2(BOOL, IsXDigit,
    AROS_LHA(struct Locale *, locale, A0),
    AROS_LHA(ULONG          , character, D0),
    struct LocaleBase *, LocaleBase, 24, Locale)
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct LocaleBase *,LocaleBase)

    return AROS_UFC2(BOOL, locale->LanguageFunction[14],
	AROS_UFCA(ULONG,    character, D0),
	AROS_UFCA(struct LocaleBase *, LocaleBase, A6));

    AROS_LIBFUNC_EXIT
} /* IsXDigit */
