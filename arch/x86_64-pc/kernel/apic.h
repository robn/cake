/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id: apic.h,v 1.1 2004/01/02 22:23:57 nicja Exp $

    Desc: AROS APIC Definitions.
    Lang: english
*/
#ifndef __AROS_APIC_H__
#define __AROS_APIC_H__

#ifndef EXEC_LISTS_H
#   include <exec/lists.h>
#endif
#ifndef EXEC_SEMAPHORES
#   include <exec/semaphores.h>
#endif
#ifndef UTILITY_HOOKS_H
#   include <utility/hooks.h>
#endif

#define	  APIC_DEFAULT_PHYS_BASE    0xfee00000
#ifndef MAX_IO_APICS
#define   MAX_IO_APICS 32
#endif

/********** APIC DEFINITIONS ****************/

struct GenericAPIC
{ 
	char                        *name; 
	IPTR                        (*probe)(); 
	IPTR                        (*apic_id_registered)();
};

#endif /* __AROS_APIC_H__ */