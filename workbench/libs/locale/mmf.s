# $Id$
include $(TOP)/config/make.cfg

# BEGIN_DESC{makefile}
# This is the mmakefile for locale.library. Use it if you want to compile
# only this part of AROS.
# END_DESC{makefile}

# Sigh, this is needed because libtail.c uses <libdefs.h> not "libdefs.h"
USER_INCLUDES := -I.

# BEGIN_DESC{localmakevar}
# \item{LIBNAME} The lowercase name of the library (without the extension).
#       This is used to help derive some filenames.
#
# \item{ULIBNAME} This also contains the library name, but with the correct
#       case (the same as found in the library base name).
#
# \item{OSMODULE} The name of the file which is created when compiling to
#       a target with module files. In this library it is expansion.library.
#
# \item{FILES} This is a list of all files (without the .c) that
#       contain internal functions of the library. You do not need to include
#       the library init, function-table or end files.
#
# \item{FUNCTIONS} This is a list of all the functions that make
#       up the library. The mmakefile will strip all the files in
#       this list for which a special CPU dependant version exists.
#       This list is concatenated into the file functions.c before
#       compilation for a speed boost.
# END_DESC{localmakevar}

OBJDIR   := $(GENDIR)/$(CURDIR)
LIBNAME  := locale
ULIBNAME := Locale

OSMODULE  := locale.library
FILES     :=
FUNCTIONS := \
	closecatalog closelocale convtolower convtoupper \
	formatdate formatstring getcatalogstr getlocalestr \
	isxxxx opencataloga openlocale parsedate prefsnotify \
	strconvert strncmp

# BEGIN_DESC{localmakevar}
# \item{DEPLIBS} List of extra libraries that you want included during
#       the linking process. You should not include the paths in this
#       list, as it will be added later. You do not need to include your
#       own lib here. If your module will be linked again after creation
#       to create a monolithic kernel file, you do not need to specify
#       any libs here (as they will be added later). If you want the
#       module to be separate (like AROSfA) you should specify the
#       libraries.
# END_DESC{localmakevar}

ifeq ("$(FLAVOUR)","native")
DEPLIBS :=
else
DEPLIBS :=
endif

# Uncomment NO_FUNCTABLE line if you don't want the function table made
# NO_FUNCTABLE := 1
%define_libs prelibs=-l$(LIBNAME)
%genlib_cpak

#MM workbench-libs-locale : setup includes linklibs
workbench-libs-locale : show-flags $(SLIB)

#MM
setup :
	%mkdirs_q $(OBJDIR) $(LIBDIR) $(SLIBDIR) $(DESTDIRS)

#MM
clean ::
	$(RM) $(OBJDIR) *.err libdefs.h functable.c functions.* mmakefile \
	$(END_FILE).c $(LIB) $(SLIB)

$(OBJDIR)/%.d : %.c
	%mkdepend_q

%asm_rule "$(FUNCTIONS) $(INIT_FILE) $(FILES) $(END_FILE)"
%ctoasm_q

%common
%include_deps $(foreach f, $(INIT_FILE) $(END_FILE) $(FILES) functions,$(OBJDIR)/$(f).d)
