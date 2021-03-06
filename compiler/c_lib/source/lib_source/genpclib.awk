BEGIN {
    stderr="/dev/stderr";

    if (file == "")
    {
        file = "libdefs.h";
    }
    while ((getline < file) > 0)
    {
        if ($2 == "BASENAME")
        {
            lib = $3;
            basename = $3;
        }
        else if ($2 == "LIBBASE")
        {
            libbase = $3;
        }
        else if ($2 == "LIBBASETYPEPTR")
        {
            libbtp = $3;
            for (t=4; t<=NF; t++)
                libbtp=libbtp" "$t;
        }
        else if ($2 == "NT_TYPE")
        {
            if( $3 == "NT_RESOURCE" )
            {
                firstlvo = 0;
                libext = ".resource";
            }
            else if( $3 == "NT_DEVICE" )
            {
                firstlvo = 6;
                libext = ".device";
            }
            else
            {
                firstlvo = 4;
                libext = ".library";
            }
        }
    }

    verbose_pattern = libbase"[ \\t]*,[ \\t]*[0-9]+[ \\t]*,[ \\t]*"basename;

    close (file);

    BASENAME=toupper(basename);
    _basename=tolower(basename);

    print "#ifndef "BASENAME"_PRIVATE_PROTOS_H"
    print "#define "BASENAME"_PRIVATE_PROTOS_H"
    print ""
    print "/*"
    print "    Copyright � 1995-2003, The AROS Development Team. All rights reserved."
    print "    *** Automatically generated by genpclib.awk. Do not edit ***"
    print ""
    print "    Desc: Private Prototypes for "basename libext
    print "    Lang: english"
    print "*/"
    print ""
    print "#include <proto/"_basename".h>"
    print ""

    file = "headers.tmpl"
    emit        = 0;
    doInsertFct = 0;

    print "/* Private Prototypes */"
}
/AROS_LH[0-9]/ { isprivate = 0; }
/AROS_((LHA)|(LHQUAD)|(PLH[0-9]))/ {

    line=$0;
    
    isarg=match(line,/AROS_LHA/);
    if(match(line,/AROS_PLH[0-9]/))
        isprivate = 1;
    gsub(/AROS_PLH/,"AROS_LP",line);
    gsub(/AROS_LH/,"AROS_LP",line);
    gsub(/^[ \t]+/,"",line);
    if (!isarg)
    {
        call=line;
        narg=0;
    }
    else
    {
        arg[narg++]=line;
    }
}
/LIBBASE[ \t]*,[ \t]*[0-9]+/ || $0 ~ verbose_pattern {
    line=$0;
    gsub(/LIBBASETYPEPTR/,libbtp,line);
    gsub(/LIBBASE/,libbase,line);
    gsub(/BASENAME/,basename,line);
    gsub(/[ \t]*[)][ \t]*$/,"",line);
    gsub(/^[ \t]+/,"",line);
    na=split(line,a,",");
    lvo=int(a[3]);

    if (lvo > firstlvo && isprivate)
    {
        print call

        for (t=0; t<narg; t++)
            print "\t"arg[t]
        print "\t"line")";

        if(doInsertFct) print insert_post
        print ""
    }
    narg=0;
}
END {
    print "#endif /* "BASENAME"_PRIVATE_PROTOS_H */"
}
