/*
    Copyright � 1995-2008, The AROS Development Team. All rights reserved.

    Desc: global include for genmodule. Defines global variables and
          the function prototypes.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "functionhead.h"
#include "config.h"

void readref(struct config *);
void writemakefile(struct config *);
void writeincproto(struct config *);
void writeincclib(struct config *);
void writeincdefines(struct config *);
void writeinclibdefs(struct config *);
void writestart(struct config *);
void writeend(struct config *);
void writeautoinit(struct config *);
void writestubs(struct config *);
void writefunclist(struct config *);
