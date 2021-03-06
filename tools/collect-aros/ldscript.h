const static char LDSCRIPT_PART1[] = 
"/*\n"
"    Script for final linking of AROS executables.\n"
"\n"
"    NOTE: This file is the result of a rearrangement of the built-in ld script.\n"
"          It's AROS-specific, in that it does constructors/destructors collecting\n"
"          and doesn't care about some sections that are not used by AROS at the moment\n"
"          or will never be.\n"
"\n"
"          It *should* be general enough to be used on many architectures.\n"
"*/\n"
"\n"
"FORCE_COMMON_ALLOCATION\n" \
"\n" \
"SECTIONS\n"
"{\n"
"  .text 0 :\n"
"  {\n"
"    *(.aros.startup)\n"
"    *(.text)\n"
"    *(.text.*)\n"
"    *(.stub)\n"
"    /* .gnu.warning sections are handled specially by elf32.em.  */\n"
"    *(.gnu.warning)\n"
"    *(.gnu.linkonce.t.*)\n"
"  } =0x90909090\n"
"\n"
"  .rodata  0 :\n"
"  {\n"
"    *(.rodata)\n"
"    *(.rodata.*)\n"
"    *(.gnu.linkonce.r.*)\n"
"  }\n"
"  .rodata1 0 : { *(.rodata1) }\n"
"\n"
"  /*\n"
"     Used only on PPC.\n"
"\n"
"     NOTE: these should go one after the other one, so some tricks\n"
"           must be used in the ELF loader to satisfy that requirement\n"
"  */\n"
"  .sdata2  0 : { *(.sdata2) *(.sdata2.*) *(.gnu.linkonce.s2.*) }\n"
"  .sbss2   0 : { *(.sbss2)  *(.sbss2.*)  *(.gnu.linkonce.sb2.*) }\n"
"\n"
"  .data  0 :\n"
"  {\n"
"    *(.data)\n"
"    *(.data.*)\n"
"    *(.gnu.linkonce.d.*)\n"
"    . = ALIGN(0x10);\n";


static const char LDSCRIPT_PART2[] =
"  }\n"
"  .data1            0 : { *(.data1) }\n"
"  .eh_frame         0 : { KEEP (*(.eh_frame)) }\n"
"  .gcc_except_table 0 : { *(.gcc_except_table) }\n"
"\n"
"  /* We want the small data sections together, so single-instruction offsets\n"
"     can access them all, and initialized data all before uninitialized, so\n"
"     we can shorten the on-disk segment size.  */\n"
"  .sdata   0 :\n"
"  {\n"
"    *(.sdata)\n"
"    *(.sdata.*)\n"
"    *(.gnu.linkonce.s.*)\n"
"  }\n"
"\n"
"  .sbss 0 :\n"
"  {\n"
"    *(.sbss)\n"
"    *(.sbss.*)\n"
"    *(.gnu.linkonce.sb.*)\n"
"    *(.scommon)\n"
"  }\n"
"\n"
"  .bss 0 :\n"
"  {\n"
"   *(.bss)\n"
"   *(.bss.*)\n"
"   *(.gnu.linkonce.b.*)\n"
"   *(COMMON)\n"
"  }\n"
"  /DISCARD/ : { *(.note.GNU-stack) }\n";

static const char LDSCRIPT_PART3[] =
"}\n";
