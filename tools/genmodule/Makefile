#   Copyright � 2002, The AROS Development Team. All rights reserved.
#   $Id$
#
#   Desc: Makefile to create genmodule
#

USER_CFLAGS := -Wall -Wunused -O2

-include $(TOP)/config/make.cfg
include Makefile.deps

HOST_CC	       ?= gcc
HOST_STRIP     ?= strip
MECHO	       ?= echo
HOST_CFLAGS    ?= $(USER_CFLAGS)
GENMODULE      ?= genmodule

all : $(GENMODULE)

GENMODULEINCLUDE := $(dir $(GENMODULE))/genmod_inc/aros/libcall.h

$(GENMODULE) : archspecific.h $(GENMODULE_DEPS) $(GENMODULEINCLUDE)
	@$(MECHO) "Compiling $(notdir $@)..."
	@$(HOST_CC) -g $(HOST_CFLAGS) -I$(GENINCDIR) $(GENMODULE_SRCS) -o $@
#	@$(HOST_STRIP) $@

$(GENMODULEINCLUDE) : include/aros/libcall.h
	@$(MKDIR) $(dir $@)
	@$(CP) $< $@

archspecific.h: archspecific.c $(AROS_INCLUDES)/aros/$(AROS_TARGET_CPU)/cpu.h
	@$(TARGET_CC) -S $<
	@grep "^[/#][*d]" archspecific.s | sed s/@SED@[$$]*//g >$@
	@rm archspecific.s

clean :
	@$(RM) $(GENMODULE) $(GENMODULEINCLUDE)

.PHONY : all clean
