#ifndef _CLASSES_FAMILY_H
#define _CLASSES_FAMILY_H

/* 
    Copyright � 1999, David Le Corfec.
    Copyright � 2002-2003, The AROS Development Team.
    All rights reserved.

    $Id$
*/

/*** Name *******************************************************************/
#define MUIC_Family            "Family.mui"

/*** Identifier base (for Zune extensions) **********************************/
#define MUIB_Family            (MUIB_ZUNE | 0x00000c00)

/*** Methods ****************************************************************/
#define MUIM_Family_AddHead    (MUIB_MUI|0x0042e200) /* MUI: V8  */
#define MUIM_Family_AddTail    (MUIB_MUI|0x0042d752) /* MUI: V8  */
#define MUIM_Family_Insert     (MUIB_MUI|0x00424d34) /* MUI: V8  */
#define MUIM_Family_Remove     (MUIB_MUI|0x0042f8a9) /* MUI: V8  */
#define MUIM_Family_Sort       (MUIB_MUI|0x00421c49) /* MUI: V8  */
#define MUIM_Family_Transfer   (MUIB_MUI|0x0042c14a) /* MUI: V8  */
struct MUIP_Family_AddHead     {STACKED ULONG MethodID; STACKED Object *obj;};
struct MUIP_Family_AddTail     {STACKED ULONG MethodID; STACKED Object *obj;};
struct MUIP_Family_Insert      {STACKED ULONG MethodID; STACKED Object *obj; STACKED Object *pred;};
struct MUIP_Family_Remove      {STACKED ULONG MethodID; STACKED Object *obj;};
struct MUIP_Family_Sort        {STACKED ULONG MethodID; STACKED Object *obj[1];};
struct MUIP_Family_Transfer    {STACKED ULONG MethodID; STACKED Object *family;};

/*** Attributes *************************************************************/
#define MUIA_Family_Child      (MUIB_MUI|0x0042c696) /* MUI: V8  i.. Object *          */
#define MUIA_Family_List       (MUIB_MUI|0x00424b9e) /* MUI: V8  ..g struct MinList *  */


extern const struct __MUIBuiltinClass _MUI_Family_desc; /* PRIV */

#endif /* _CLASSES_FAMILY_H */
