#ifndef DOS_BPTR_H
#define DOS_BPTR_H

/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$

    Desc: AROS version of BPTRs
    Lang: english
*/
#ifndef EXEC_TYPES_H
#   include <exec/types.h>
#endif
#ifndef AROS_MACHINE_H
#   include <aros/machine.h>
#endif

/*
    Replace BPTRs by simple APTRs for some machines. On Amiga with binary
    compatibility, this would look like this:

    typedef ULONG BPTR;
    #define MKBADDR(a)      (((BPTR)(a))>>2)
    #define BADDR(a)        (((APTR)(a))<<2)
*/
#ifndef AROS_BPTR_TYPE
#   define AROS_FAST_BPTR
#   define AROS_BPTR_TYPE   APTR
#   define MKBADDR(a)       ((APTR)(a))
#   define BADDR(a)         (a)
#endif
#ifndef AROS_BSTR_TYPE
#   define AROS_BSTR_TYPE   STRPTR
#endif

/* Macros to transparently handle BSTRs */
#ifdef AROS_FAST_BPTR
#   define AROS_BSTR_ADDR(s)        ((STRPTR)(s))
#   define AROS_BSTR_strlen(s)      (strlen (s))
#   define AROS_BSTR_setstrlen(s,l) (s[l] = 0)
#   define AROS_BSTR_getchar(s,l)   (s[l])
#   define AROS_BSTR_putchar(s,l,c) (s[l] = c)
#else
#   define AROS_BSTR_ADDR(s)        ((STRPTR)BADDR(s))
#   define AROS_BSTR_strlen(s)      (AROS_BSTR_ADDR(s)[0])
#   define AROS_BSTR_setstrlen(s,l) (AROS_BSTR_ADDR(s)[0] = l)
#   define AROS_BSTR_getchar(s,l)   (AROS_BSTR_ADDR(s)[l])
#   define AROS_BSTR_putchar(s,l,c) (AROS_BSTR_ADDR(s)[l] = c)
#endif

#ifndef __typedef_BPTR
#   define __typedef_BPTR
    typedef AROS_BPTR_TYPE BPTR;
#endif
#ifndef __typedef_BSTR
#   define __typedef_BSTR
    typedef AROS_BSTR_TYPE BSTR;
#endif

#endif /* DOS_BPTR_H */
