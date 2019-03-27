/*
 * Copyright (C) 2019  Red Hat, Inc.
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

/* inspect_kernel.c */
bool compare_module_parameters(const struct kmod_list *, const struct kmod_list *, string_list_t **);
bool compare_module_dependencies(const struct kmod_list *, const struct kmod_list *, string_list_t **, string_list_t **);

struct kernel_alias_data;
typedef void (*module_alias_callback)(const char *, const string_list_t *, const string_list_t *, void *);

void gather_module_aliases(const char *, const struct kmod_list *, struct kernel_alias_data **);
void free_module_aliases(struct kernel_alias_data *);
bool compare_module_aliases(struct kernel_alias_data *, struct kernel_alias_data *, module_alias_callback, void *);

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
bool inspect_manpage_alloc(void);
void inspect_manpage_free(void);
bool inspect_manpage_path(const char *);
char * inspect_manpage_validity(const char *);
bool inspect_manpage(struct rpminspect *);

/* inspect_metadata.c */
bool inspect_metadata(struct rpminspect *);

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

#endif
