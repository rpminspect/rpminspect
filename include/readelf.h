/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
 * Red Hat Author(s):  David Shea <dshea@redhat.com>
 *                     David Cantrell <dcantrell@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _READELF_H
#define _READELF_H

#include <stdbool.h>
#include <libelf.h>
#include <gelf.h>
#include <stdbool.h>
#include <stdint.h>
#include <elf.h>

#include "types.h"

/* Older systems might lack this definition */
#ifndef EM_BPF
#define EM_BPF 247 /* Linux BPF -- in-kernel virtual machine */
#endif

Elf *get_elf(const char *, int *);
Elf *get_elf_archive(const char *, int *);
GElf_Half get_elf_type(Elf *);
GElf_Half get_elf_machine(Elf *, int);
bool is_elf(const char *);
bool have_elf_section(Elf *, int64_t, const char *);
string_list_t *get_elf_section_names(Elf *elf, size_t start);
Elf_Scn *get_elf_section(Elf *, int64_t, const char *, Elf_Scn *, GElf_Shdr *);
Elf_Scn *get_elf_extended_section(Elf *, Elf_Scn *, GElf_Shdr *);
GElf_Phdr *get_elf_phdr(Elf *, Elf64_Word, GElf_Phdr *);
char *get_elf_soname(const char *);

bool have_dynamic_tag(Elf *, const Elf64_Sxword);
bool get_dynamic_tags(Elf *, const Elf64_Sxword, GElf_Dyn **, size_t *, GElf_Shdr *);

string_list_t *get_elf_imported_functions(Elf *, bool (*)(const char *));
string_list_t *get_elf_exported_functions(Elf *, bool (*)(const char *));

typedef bool (*elf_ar_action)(Elf *, string_list_t **);
void elf_archive_iterate(int, Elf *, elf_ar_action, string_list_t **);

#endif
