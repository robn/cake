/*
    Copyright � 1995-2003, The AROS Development Team. All rights reserved.
    $Id$
    
    Function to write modulename_mcc_init.c. Part of genmodule.
*/
#include "genmodule.h"

void writemccinit(void)
{
    FILE *out;
    char line[256];
    struct functionlist *methlistit;
    struct arglist *arglistit;
    struct linelist *linelistit;
    unsigned int lvo;
    int i;
    
    snprintf(line, 255, "%s/%s_mcc_init.c", gendir, modulename);
    out = fopen(line, "w");
    if(out==NULL)
    {
        fprintf(stderr, "Could not write %s\n", line);
        exit(20);
    }
    
    fprintf
    (
        out,
        "/*\n"
        "    *** Automatically generated file. Do not edit! ***\n"
        "    Copyright � 1995-2003, The AROS Development Team. All rights reserved.\n"
        "*/\n"
        "\n"
        "#include <exec/types.h>\n"
        "#include <exec/libraries.h>\n"
        "#include <dos/dosextens.h>\n"
        "#include <aros/libcall.h>\n"
        "#include <aros/debug.h>\n"
        "#include <libcore/base.h>\n"
        "\n"
        "#include <intuition/classes.h>\n"
        "#include <intuition/classusr.h>\n"
        "\n"
        "#include <proto/exec.h>\n"
        "#include <proto/utility.h>\n"
        "#include <proto/dos.h>\n"
        "#include <proto/graphics.h>\n"
        "#include <proto/intuition.h>\n"
        "#include <proto/muimaster.h>\n"
        "\n"
        "#include <aros/symbolsets.h>\n"
        "#include LC_LIBDEFS_FILE\n"
        "\n"
    );
        
    for(linelistit = cdeflines; linelistit != NULL; linelistit = linelistit->next)
    {
        fprintf(out, "%s\n", linelistit->line);
    }

    fprintf
    (
        out,
        "\n"
        "/*** Instance data structure size ***************************************/\n"
        "#ifndef NO_CLASS_DATA\n"
        "#   define %s_DATA_SIZE (sizeof(struct %s_DATA))\n"
        "#else\n"
        "#   define %s_DATA_SIZE (0)\n"
        "#endif\n",
        modulename, modulename, modulename
    );
    
    fprintf
    (
        out,
        "\n"
        "\n"
        "/*** Variables **************************************************************/\n"
        "struct ExecBase        *SysBase;\n"
        "struct UtilityBase     *UtilityBase;\n"
        "struct DosLibrary      *DOSBase;\n"
        "struct GfxBase         *GfxBase;\n"
        "struct IntuitionBase   *IntuitionBase;\n"
        "struct Library         *MUIMasterBase;\n"
        "\n"
        "struct MUI_CustomClass *MCC;\n"
        "\n"
        "\n"
        "/*** Prototypes *************************************************************/\n"
    );
    
    for 
    (
        methlistit = methlist; 
        methlistit != NULL; 
        methlistit = methlistit->next)
    {
        int first = 1;
        
        fprintf(out, "%s %s__%s(", methlistit->type, modulename, methlistit->name);
        
        for 
        (
            arglistit = methlistit->arguments; 
            arglistit != NULL; 
            arglistit = arglistit->next)
        {
            if (!first)
                fprintf(out, ", ");
            else
                first = 0;
            
            fprintf(out, "%s %s", arglistit->type, arglistit->name );
        }
        
        fprintf(out, ");\n");
    }

    if (!customdispatcher)
    {
        fprintf
        (
            out,
            "\n"
            "\n"
            "/*** Dispatcher *************************************************************/\n"
            "BOOPSI_DISPATCHER(IPTR, %s_Dispatcher, CLASS, self, message)\n"
            "{\n"
            "    switch (message->MethodID)\n"
            "    {\n",
            modulename
        );
        
        for 
        (
            methlistit = methlist; 
            methlistit != NULL; 
            methlistit = methlistit->next)
        {
            fprintf
            (
                out, 
                "        case %s: return (IPTR) %s__%s(", 
                methlistit->name, modulename, methlistit->name
            );
            
            if (methlistit->argcount != 3)
            {
                fprintf(stderr, "Method \"%s\" has wrong number of arguments\n", methlistit->name);
                exit(20);
            }
            
            arglistit = methlistit->arguments;
            fprintf(out, "CLASS, ");
            arglistit = arglistit->next;
            fprintf(out, "self, ");
            arglistit = arglistit->next;
            fprintf(out, "(%s) message);\n", arglistit->type);
        }
        
        fprintf
        (
            out,
            "        default: return DoSuperMethodA(CLASS, self, message);\n"
            "    }\n"
            "    \n"
            "    return (IPTR) NULL;\n"
            "}\n"
            "BOOPSI_DISPATCHER_END\n"
        );
    }
    else
    {
        fprintf
        (
            out,
            "\n"
            "\n"
            "/*** Custom dispatcher prototype ********************************************/\n"
            "BOOPSI_DISPATCHER_PROTO(IPTR, %s_Dispatcher, CLASS, object, message);\n",
            modulename
        );
    }
    
    fprintf
    (
        out,
        "\n"
        "\n"
        "/*** Library startup and shutdown *******************************************/\n"
        "AROS_SET_LIBFUNC(MCC_Startup, LIBBASETYPE, LIBBASE)\n"
        "{\n"
        "    AROS_SET_LIBFUNC_INIT\n"
        "    \n"
        "    SysBase = LIBBASE->lh_SysBase;\n"
        "    \n"
        "    UtilityBase   = NULL;\n"
        "    DOSBase       = NULL;\n"
        "    GfxBase       = NULL;\n"
        "    IntuitionBase = NULL;\n"
        "    MUIMasterBase = NULL;\n"
        "    \n"
        "    UtilityBase = (struct UtilityBase *) OpenLibrary(\"utility.library\", 0);\n"
        "    if (UtilityBase == NULL) goto error;\n"
        "    \n"
        "    DOSBase = (struct DosLibrary *) OpenLibrary(\"dos.library\", 0);\n"
        "    if (DOSBase == NULL) goto error;\n"
        "    \n"
        "    GfxBase = (struct GfxBase *) OpenLibrary(\"graphics.library\", 0);\n"
        "    if (GfxBase == NULL) goto error;\n"
        "    \n"
        "    IntuitionBase = (struct IntuitionBase *) OpenLibrary(\"intuition.library\", 0);\n"
        "    if (IntuitionBase == NULL) goto error;\n"
        "    \n"
        "    MUIMasterBase = OpenLibrary(\"muimaster.library\", 0);\n"
        "    if (MUIMasterBase == NULL) goto error;\n"
        "    \n"
        "    MCC = MUI_CreateCustomClass((struct Library *) LIBBASE, \"%s\", NULL, %s_DATA_SIZE, %s_Dispatcher);\n"
        "    if (MCC == NULL) goto error;\n"
        "    \n"
        "    return TRUE;\n"
        "\n"
        "error:\n"
        "    if (MUIMasterBase != NULL) CloseLibrary(MUIMasterBase);\n"
        "    if (IntuitionBase != NULL) CloseLibrary((struct Library *) IntuitionBase);\n"
        "    if (GfxBase != NULL) CloseLibrary((struct Library *) GfxBase);\n"
        "    if (DOSBase != NULL) CloseLibrary((struct Library *) DOSBase);\n"
        "    if (UtilityBase != NULL) CloseLibrary((struct Library *) UtilityBase);\n"
        "    \n"
        "    return FALSE;\n"
        "    \n"
        "    AROS_SET_LIBFUNC_EXIT\n"
        "}\n"
        "\n"
        "AROS_SET_LIBFUNC(MCC_Shutdown, LIBBASETYPE, LIBBASE)\n"
        "{\n"
        "    AROS_SET_LIBFUNC_INIT\n"
        "    \n"
        "    if (MUIMasterBase != NULL) CloseLibrary(MUIMasterBase);\n"
        "    if (IntuitionBase != NULL) CloseLibrary((struct Library *) IntuitionBase);\n"
        "    if (GfxBase != NULL) CloseLibrary((struct Library *) GfxBase);\n"
        "    if (DOSBase != NULL) CloseLibrary((struct Library *) DOSBase);\n"
        "    if (UtilityBase != NULL) CloseLibrary((struct Library *) UtilityBase);\n"
        "    \n"
        "    MUI_DeleteCustomClass(MCC);\n"
        "    return TRUE;\n"
        "    \n"
        "    AROS_SET_LIBFUNC_EXIT\n"
        "}\n"
        "\n"
        "ADD2INITLIB(MCC_Startup, 0);\n"
        "ADD2EXPUNGELIB(MCC_Shutdown, 0);\n",
        superclass, modulename, modulename
    );
    
    fclose(out);
}
