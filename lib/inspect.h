/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
 * Author(s):  David Cantrell <dcantrell@redhat.com>
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

/*
 * This header defines the test functions provided by librpminspect.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <libelf.h>
#include <libkmod.h>

#include "types.h"

#ifndef _LIBRPMINSPECT_INSPECT_H
#define _LIBRPMINSPECT_INSPECT_H

/*
 * Inspection drivers (and their helpers)
 *
 * All inspection drivers use the same prototype.  They return a bool
 * and take one parameter:  a struct rpminspect pointer.  Inspection
 * drivers are named as "inspect_NAME" where NAME is the short name
 * specified in the inspections array (see inspect.c).  The source
 * code filename should also match the driver function name.
 *
 * Add any helper functions you need in order to keep the inspection
 * driver simple.
 */

/* inspect.c */
typedef bool (*foreach_peer_file_func)(struct rpminspect *, rpmfile_entry_t *);
bool foreach_peer_file(struct rpminspect *, foreach_peer_file_func);

/* inspect_elf.c */
void init_elf_data(void);
void free_elf_data(void);
bool has_executable_program(Elf *);
bool is_execstack_present(Elf *);
uint64_t get_execstack_flags(Elf *);
bool is_execstack_valid(Elf *, uint64_t);
bool is_stack_executable(Elf *, uint64_t);
bool has_textrel(Elf *);
bool has_relro(Elf *);
bool has_bind_now(Elf *);
string_list_t * get_fortified_symbols(Elf *);
string_list_t * get_fortifiable_symbols(Elf *);
bool is_pic_ok(Elf *);
bool inspect_elf(struct rpminspect *);

/* inspect_license.c */
void free_licensedb(void);
bool is_valid_license(const char *, const char *);
bool inspect_license(struct rpminspect *);

/* inspect_emptyrpm.c */
bool is_payload_empty(rpmfile_t *);
bool inspect_emptyrpm(struct rpminspect *);

/* inspect_xml.c */
bool is_xml_well_formed(const char *, char **);
bool inspect_xml(struct rpminspect *);

/* inspect_manpage.c */
bool inspect_manpage(struct rpminspect *);

/* inspect_metadata.c */
bool inspect_metadata(struct rpminspect *);

/* inspect_desktop.c */
bool inspect_desktop(struct rpminspect *);

/* inspect_disttag.c */
bool inspect_disttag(struct rpminspect *);

/* inspect_specname.c */
bool inspect_specname(struct rpminspect *);

/* inspect_modularity.c */
bool inspect_modularity(struct rpminspect *);

/* inspect_javabytecode.c */
bool inspect_javabytecode(struct rpminspect *);

/* inspect_changedfiles.c */
bool inspect_changedfiles(struct rpminspect *);

/* inspect_removedfiles.c */
bool inspect_removedfiles(struct rpminspect *);

/* inspect_addedfiles.c */
bool inspect_addedfiles(struct rpminspect *);

/* inspect_upstream.c */
bool inspect_upstream(struct rpminspect *);

/* inspect_ownership.c */
bool inspect_ownership(struct rpminspect *);

/* inspect_shellsyntax.c */
bool inspect_shellsyntax(struct rpminspect *);

/* inspect_annocheck.c */
bool inspect_annocheck(struct rpminspect *);

/* inspect_dt_needed.c */
bool inspect_dt_needed(struct rpminspect *);

/* inspect_filesize.c */
bool inspect_filesize(struct rpminspect *);

/* inspect_permissions.c */
bool inspect_permissions(struct rpminspect *);

/* inspect_capabilities.c */
bool inspect_capabilities(struct rpminspect *);

/* inspect_kmod.c */
bool inspect_kmod(struct rpminspect *);

/* inspect_arch.c */
bool inspect_arch(struct rpminspect *);

/*
 * Inspections are referenced by flag.  These flags are set in bitfields
 * to indicate which ones we want to run.  When adding new ones, please
 * follow the existing convention.  Inspection names should be short but
 * descriptive.  Note that inspection names need to specified on the
 * command line.
 */

#define INSPECT_LICENSE                     (((uint64_t) 1) << 1)
#define INSPECT_EMPTYRPM                    (((uint64_t) 1) << 2)
#define INSPECT_METADATA                    (((uint64_t) 1) << 3)
#define INSPECT_MANPAGE                     (((uint64_t) 1) << 4)
#define INSPECT_XML                         (((uint64_t) 1) << 5)
#define INSPECT_ELF                         (((uint64_t) 1) << 6)
#define INSPECT_DESKTOP                     (((uint64_t) 1) << 7)
#define INSPECT_DISTTAG                     (((uint64_t) 1) << 8)
#define INSPECT_SPECNAME                    (((uint64_t) 1) << 9)
#define INSPECT_MODULARITY                  (((uint64_t) 1) << 10)
#define INSPECT_JAVABYTECODE                (((uint64_t) 1) << 11)
#define INSPECT_CHANGEDFILES                (((uint64_t) 1) << 12)
#define INSPECT_REMOVEDFILES                (((uint64_t) 1) << 13)
#define INSPECT_ADDEDFILES                  (((uint64_t) 1) << 14)
#define INSPECT_UPSTREAM                    (((uint64_t) 1) << 15)
#define INSPECT_OWNERSHIP                   (((uint64_t) 1) << 16)
#define INSPECT_SHELLSYNTAX                 (((uint64_t) 1) << 17)
#define INSPECT_ANNOCHECK                   (((uint64_t) 1) << 18)
#define INSPECT_DT_NEEDED                   (((uint64_t) 1) << 19)
#define INSPECT_FILESIZE                    (((uint64_t) 1) << 20)
#define INSPECT_PERMISSIONS                 (((uint64_t) 1) << 21)
#define INSPECT_CAPABILITIES                (((uint64_t) 1) << 22)
#define INSPECT_KMOD                        (((uint64_t) 1) << 23)
#define INSPECT_ARCH                        (((uint64_t) 1) << 24)

#endif
