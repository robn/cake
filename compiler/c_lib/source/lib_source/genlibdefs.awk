#
# Create libdefs.h from a file lib.conf. lib.conf may contain these options:
#
# name <string> - Init the various fields with reasonable defaults. If
#		<string> is XXX, then this is the result:
#
#		    libname	    xxx
#		    basename	    Xxx
#		    libbase	    XxxBase
#		    libbasetype     struct XxxBase
#		    libbasetypeptr  struct XxxBase *
#
#		Variables will only be changed if they have not yet been
#		specified.
#
# libname <string> - Set libname to <string>. This is the name of the
#		library (ie. you can open it with <string>.library).
#		It will show up in the version string, too.
# basename <string> - Set basename to <string>. The basename is used in
#		the AROS-LHx macros in the location part (last parameter)
#		and to specify defaults for libbase and libbasetype
#		in case they have no value yet. If <string> is xXx, then
#		libbase will become xXxBase and libbasetype will become
#		xXxBase.
# libbase <string> - Defines the name of the library base (ie. SysBase,
#		DOSBase, IconBase, etc). If libbasetype is not set, then
#		it is set to <string>, too.
# libbasetype <string> - The type of libbase (with struct), ie.
#		struct ExecBase, struct DosLibrary, struct IconBase, etc).
# libbasetypeptr <string> - Type of a pointer to the libbase. (eg.
#		struct ExecBase *).
# version <version>.<revision> - Specifies the version and revision of the
#		library. 41.0103 means version 41 and revision 103.
# copyright <string> - Copyright string.
# define <string> - The define to use to protect the resulting file
#		against double inclusion (ie. #ifndef <string>...)
#		The default is _LIBDEFS_H.
# type <string> - What kind of library is this ? Valid values
#		for <string> are: device, library, resource and hidd.
# option <string>... - Specify an option. Valid values for <string> are:
#
#		    noexpunge - Once the lib/dev is loaded, it can't be
#				removed from memory. Be careful with this
#				option.
#		    rom - For ROM based libraries. Implies noexpunge and
#				unique.
#		    unique - Generate unique names for all external
#				symbols.
#		    nolibheader - We don't want to use the LibHeader prefixed
#				functions in the function table.
#		    hasrt - This library has resource tracking.
#
#		You can specify more than one option in a config file and
#		more than one option per option line. Separate options by
#		space.
#
BEGIN {
    libbase="";
    libbasetype="";
    basename="";
    define="_LIBDEFS_H";
    type = "library";
}
/^name/ {
    libname=tolower($2);
    if (basename=="")
	basename=toupper(substr(libname,1,1)) substr(libname,2);
    if (libbase=="")
	libbase=basename"Base";
    if (libbasetype=="")
	libbasetype="struct "basename"Base";
}
/^libname/ { libname=$2; }
/^basename/ {
    basename=$2;
    if (libbase=="")
	libbase=basename"Base";
    if (libbasetype=="")
	libbasetype="struct "basename"Base";
}
/^libbase[ \t]+/ {
    libbase=$2;
    if (libbasetype=="")
	libbasetype="struct "libbase;
}
/^libbasetype/ {
    libbasetype=$2;
    for (t=3; t<=NF; t++)
	libbasetype=libbasetype" "$t;
}
/^libbasetypeptr/ {
    libbasetypeptr=$2;
    for (t=3; t<=NF; t++)
	libbasetypeptr=libbasetypeptr" "$t;
}
/^define/ {
    define=$2;
}
/^version/ {
    str=$2;
    n=split(str,a,".");
    version=int(a[1]);
    revision=int(a[2]);
}
/^copyright/ {
    match ($0,/^copyright[ \t]+/);
    copyright = substr ($0,RSTART+RLENGTH);
}
/^type/ {
    type = $2;
}
/^option/ {
    for (t=2; t<=NF; t++)
    {
	if ($t == "noexpunge")
	    noexpunge = 1;
	else if ($t == "rom")
	    rom = 1;
	else if ($t == "unique")
	    unique = 1;
	else if ($t == "nolibheader")
	    nolibheader = 1;
	else if ($t == "hasrt")
	    hasrt = 1;
	else
	{
	    print "Unknown option \"" $t "\"" > "/dev/stderr";
	    exit (10);
	}
    }
}
END {
    cmd="date \"+%d.%m.%Y\"";
    cmd | getline date;

    if (libbase=="")
	libbase=basename"Base";
    if (libbasetype=="")
	libbasetype="struct "basename"Base";
    if (libbasetypeptr=="")
	libbasetypeptr=libbasetype" *";

    print "/*** Automatically generated by genlibdefs.awk. Do not edit ***/"
    print "#ifndef "define"\n#define "define

    if (type == "library")
    {
	print "#define NAME_STRING      \""libname".library\""
	print "#define NT_TYPE          NT_LIBRARY"
    }
    else if (type == "device")
    {
	print "#define NAME_STRING      \""libname".device\""
	print "#define NT_TYPE          NT_DEVICE"
    }
    else if (type == "resource")
    {
	print "#define NAME_STRING      \""libname".resource\""
	print "#define NT_TYPE          NT_RESOURCE"
    }
    else if (type == "hidd")
    {
	print "#define NAME_STRING      \""libname".hidd\""
	print "#define NT_TYPE          NT_HIDD"
    }
    else if (type == "datatype")
    {
	print "#define NAME_STRING	\""libname".datatype\""
	print "#define NT_TYPE		NT_LIBRARY"
    }
    else if (type == "gadget")
    {
	print "#define NAME_STRING	\""libname".gadget\""
	print "#define NT_TYPE		NT_LIBRARY"
    }
    else
    {
	print "Unknown type "type > "/dev/stderr";
	exit (10);
    }

    if (rom)
	noexpunge = 1;

    if (noexpunge)
	print "#define NOEXPUNGE"
    if (rom)
	print "#define ROMBASED"
    if (nolibheader)
	print "#define NOLIBHEADER"

    if (rom || unique)
    {
	print "#define LC_UNIQUE_PREFIX "basename
	print "#define LC_BUILDNAME(n)  "basename" ## n"
    }
    else
    {
	print "#define LC_BUILDNAME(n)  n"
    }

    print "#define LIBBASE          "libbase
    print "#define LIBBASETYPE      "libbasetype
    print "#define LIBBASETYPEPTR   "libbasetypeptr
    print "#define VERSION_NUMBER   "version
    print "#define REVISION_NUMBER  "revision
    print "#define BASENAME         "basename
    print "#define BASENAME_STRING  \""basename"\""
    print "#define VERSION_STRING   \"$VER: "libname" "version"."revision" ("date")\\r\\n\""
    print "#define LIBEND           "basename"_end"
    print "#define LIBFUNCTABLE     "basename"_functable"
    print "#define COPYRIGHT_STRING \""copyright"\""
    print "#endif /* "define" */"
}
