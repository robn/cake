/*
 Copyright (C) 2006-2009 The AROS Development Team. All rights reserved.
 $Id$
 
 Desc: ELF32 loader extracted from our internal_load_seg_elf in dos.library.
 Lang: English
 */

#include "elfloader32.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

#define D(x)
#define DREL(x)

#define SECTION_NAME(section_index)         ((char *) (image + sh[eh->shstrndx].offset + sh[section_index].name))
#define SYMBOL_NAME(symtab_index, symbol)   ((char *) (symbol->name ? (image + sh[sh[symtab_index].link].offset + symbol->name) : "<anonymous>"))

int load_elf_image(void *image, void *memory, uint32_t virt, void **start, void **end, void **entry) {
    struct elfheader *eh;
    struct sheader *sh;
    int i;
    uint32_t alloc = virt;

    D(printf("[elf] loading ELF module to virtual address %p\n", virt));

    eh = (struct elfheader *) image;
    if (eh->ident[0] != 0x7f || eh->ident[1] != 'E' || eh->ident[2] != 'L' || eh->ident[3] != 'F') {
        fprintf(stderr, "[elf] kernel image is not an ELF image\n");
        return -1;
    }
    if (eh->type != ET_REL || eh->machine != EM_386) {
        fprintf(stderr, "[elf] kernel image has wrong type or architecture\n");
        return -1;
    }

    sh = (struct sheader *) (image + eh->shoff);

    if (start != NULL)
        *start = (void *) memory + alloc;

    if (entry != NULL)
        *entry = NULL;

    for (i = 0; i < eh->shnum; i++) {
        if (sh[i].flags & SHF_ALLOC) {
            alloc = (alloc + sh[i].addralign - 1) & ~(sh[i].addralign - 1);
            sh[i].addr = (void *) memory + alloc;

            D(printf("[elf] allocating 0x%x bytes at 0x%x (alignment 0x%x) for section '%s'\n", sh[i].size, sh[i].addr, sh[i].addralign, SECTION_NAME(i)));

            switch (sh[i].type) {
                case SHT_PROGBITS:
                    D(printf("[elf] section is SHT_PROGBITS, copying 0x%x bytes\n", sh[i].size));
                    memcpy(sh[i].addr, image + sh[i].offset, sh[i].size);

                    if (sh[i].flags & SHF_EXECINSTR && entry != NULL && *entry == NULL) {
                        *entry = sh[i].addr;
                        D(printf("[elf] first executable section, entry point is 0x%x\n", *entry));
                    }

                    break;

                case SHT_NOBITS:
                    D(printf("[elf] section is SHT_NOBITS, clearing 0x%x bytes\n", sh[i].size));
                    memset(sh[i].addr, 0, sh[i].size);
                    break;

                default:
                    fprintf(stderr, "[elf] no support to allocate for type 0x%x section\n", sh[i].type);
                    return -1;
            }

            alloc += sh[i].size;
        }
    }

    if (end != NULL)
        *end = (void *) memory + alloc - 1;

    for (i = 0; i < eh->shnum; i++) {
        if (sh[i].type == SHT_REL || sh[i].type == SHT_RELA) {
            if(sh[sh[i].info].addr == NULL) {
                D(printf("[elf] section '%s' has relocations for section '%s', but that section has no allocation, skipping it\n", SECTION_NAME(i), SECTION_NAME(sh[i].info)));
                continue;
            }

            int nrelocs = sh[i].size / sh[i].entsize;

            D(printf("[elf] applying %d relocations in section '%s' to section '%s' using symbol table in section '%s'\n", nrelocs, SECTION_NAME(i), SECTION_NAME(sh[i].info), SECTION_NAME(sh[i].link)));

            int j;
            for (j = 0; j < nrelocs; j++) {
                struct relo *rel = image + sh[i].offset + sizeof(struct relo) * j;
                struct symbol *sym = image + sh[sh[i].link].offset + sizeof(struct symbol) * ELF32_R_SYM(rel->info);
                uint32_t *src = image + sh[sh[i].info].offset + rel->offset;
                uint32_t *dst = sh[sh[i].info].addr + rel->offset;
                uint32_t s;

                switch (sym->shindex) {
                    case SHN_UNDEF:
                        fprintf(stderr, "[elf] undefined symbol '%s' in section '%s', relocation failed\n", SYMBOL_NAME(sh[i].link, sym), SECTION_NAME(sh[i].info));
                        return -1;

                    case SHN_COMMON:
                        fprintf(stderr, "[elf] undefined symbol '%s' in section '%s', relocation failed\n", SYMBOL_NAME(sh[i].link, sym), SECTION_NAME(sh[i].info));
                        return -1;

                    case SHN_ABS:
                        if (strcmp(SYMBOL_NAME(sh[i].link, sym), "SysBase") == 0) {
                            fprintf(stderr, "[elf] absolute SysBase relocation wanted in section '%s'. that's bad for kernel code, and may cause a crash\n", SECTION_NAME(sh[i].info));
                            s = 0;
                        }
                        else
                            s = sym->value;
                        break;

                    default:
                        s = (uint32_t) sh[sym->shindex].addr + sym->value;
                        break;
                }

                switch (ELF32_R_TYPE(rel->info)) {
                    case R_386_32:
                        *dst = *src + s;
                        break;

                    case R_386_PC32:
                        *dst = *src + s - (uint32_t) dst;
                        break;

                    case R_386_NONE:
                        break;

                    default:
                        fprintf(stderr, "[elf] unknown relocation type 0x%02x for symbol '%s' in section '%s', relocation failed\n", ELF32_R_TYPE(rel->info), SYMBOL_NAME(sh[i].link, sym), SECTION_NAME(sh[i].info));
                        return -1;
                }

                DREL(printf("[elf] relocated symbol '%s' type %s value %p at %p\n", SYMBOL_NAME(sh[i].link, sym),
                                                                                        ELF32_R_TYPE(rel->info) == R_386_32   ? "R_386_32"   :
                                                                                        ELF32_R_TYPE(rel->info) == R_386_PC32 ? "R_386_PC32" :
                                                                                                                        "<unknown>",
                                                                                        *dst, dst));
            }
        }
    }

    return 0;
}

