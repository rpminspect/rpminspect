/*
 * Copyright (C) 2020  Red Hat, Inc.
 * Author(s):  David Cantrell <dcantrell@redhat.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file inspect_lto.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2020
 * @brief LTO bytecode inspection and helper functions.
 * @copyright LGPL-3.0-or-later
 */

#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <elf.h>
#include <rpm/header.h>
#include "queue.h"
#include "rpminspect.h"

static string_list_t *lto_symbol_name_prefixes = NULL;

/**
 * @brief Callback for lto_driver() for inspecting ELF .a files.
 *
 * An ELF static library is an ar(1) archive of ELF .o files.  This
 * callback function is used to iterate over each .o file and perform
 * the same check used when examining a single ELF relocatable .o
 * file.  Any found symbols are added the the user_data list for later
 * reporting.  The caller must free the user_data list.
 *
 * @param elf ELF file handle under examination
 * @param user_data string_list_t for results
 * @return True under normal conditions, false to indicate badness
 */
static bool find_lto_symbols(Elf *elf, string_list_t **user_data)
{
    string_list_t *specifics = *user_data;
    string_list_t *names = NULL;
    string_entry_t *entry = NULL;
    string_entry_t *prefix = NULL;
    string_entry_t *found = NULL;
    Elf_Arhdr *arhdr;

    assert(elf != NULL);

    if ((arhdr = elf_getarhdr(elf)) == NULL) {
        return true;
    }

    specifics = calloc(1, sizeof(*specifics));
    TAILQ_INIT(specifics);

    names = get_elf_section_names(elf, SHT_PROGBITS);

    if (names != NULL) {
        TAILQ_FOREACH(entry, names, items) {
            DEBUG_PRINT("entry->data=|%s|\n", entry->data);

            TAILQ_FOREACH(prefix, lto_symbol_name_prefixes, items) {
                DEBUG_PRINT("lto_symbol_name_prefix=|%s|\n", prefix->data);

                if (strprefix(entry->data, prefix->data)) {
                    /* don't add the symbol if we already have it */
                    TAILQ_FOREACH(found, specifics, items) {
                        if (!strcmp(found->data, entry->data)) {
                            break;
                        }
                    }

                    /* add this symbol to the list */
                    found = calloc(1, sizeof(*found));
                    found->data = strdup(entry->data);
                    TAILQ_INSERT_TAIL(specifics, found, items);
                    break;
                }
            }
        }
    }

    if (TAILQ_EMPTY(specifics)) {
        list_free(specifics, free);
        specifics = NULL;
    }

    list_free(names, free);

    return true;
}

/**
 * @brief Called by the main LTO inspection driver.
 *
 * If the file is in a binary RPM and is an ELF .o or .a file, check
 * to see if any of the symbols in the ELF symbol table carry an LTO
 * symbol prefix as defined in the configuration file.  If there are
 * any matches, report the findings as a BAD result and explain to the
 * user that .o and .a files should not carry LTO symbols because of
 * portability issues between compiler versions.
 *
 * @param ri The struct rpminspect pointer for the run of the program
 * @param file The file the function is asked to examine
 * @return True if the file passes, false otherwise
 */
static bool lto_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    bool result = true;
    Elf *elf = NULL;
    int fd = -1;
    string_list_t *names = NULL;
    string_entry_t *entry = NULL;
    string_entry_t *prefix = NULL;
    const char *arch = NULL;
    char *badsyms = NULL;
    struct result_params params;

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Only valid for ELF files */
    if (!is_elf(file->fullpath) && !strsuffix(file->localpath, STATIC_LIB_FILENAME_EXTENSION)) {
        return true;
    }

    /* architecture is used in reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* initialize the result parameters */
    init_result_params(&params);
    params.severity = RESULT_BAD;
    params.waiverauth = NOT_WAIVABLE;
    params.header = HEADER_LTO;
    params.remedy = REMEDY_LTO;
    params.verb = VERB_FAILED;
    params.arch = arch;
    params.file = file->localpath;

    if ((elf = get_elf_archive(file->fullpath, &fd)) != NULL) {
        /* we found an ELF static library */
        elf_archive_iterate(fd, elf, find_lto_symbols, &names);

        if (names != NULL) {
            params.noun = entry->data;
            badsyms = list_to_string(names, ", ");
            xasprintf(&params.msg, _("%s contains symbols [%s] on %s; this is not portable across compiler versions"), file->localpath, badsyms, arch);
            add_result(ri, &params);
            free(params.msg);
            free(badsyms);
            result = false;
        }
    } else if (((elf = get_elf(file->fullpath, &fd)) != NULL) && (get_elf_type(elf) == ET_REL)) {
        /* we found an ELF relocatable */
        names = get_elf_section_names(elf, SHT_SYMTAB);

        if (names != NULL) {
            TAILQ_FOREACH(entry, names, items) {
                TAILQ_FOREACH(prefix, ri->lto_symbol_name_prefixes, items) {
                    if (strprefix(entry->data, prefix->data)) {
                        params.noun = entry->data;
                        xasprintf(&params.msg, _("%s contains symbol [%s] on %s; this is not portable across compiler versions"), file->localpath, entry->data, arch);
                        add_result(ri, &params);
                        free(params.msg);
                        result = false;
                        break;
                    }
                }
            }
        }
    }

    if (elf) {
        elf_end(elf);
        close(fd);
    }

    list_free(names, free);

    return result;
}

/**
 * @brief Main driver for the 'lto' inspection.
 *
 * LTO (Link Time Optimization) bytecode is not stable from one
 * release of gcc to the next.  This inspection checks ELF .o and .a
 * files to ensure all LTO bytecode has been stripped.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_lto(struct rpminspect *ri) {
    bool result = true;
    struct result_params params;

    assert(ri != NULL);

    if (ri->lto_symbol_name_prefixes != NULL) {
        lto_symbol_name_prefixes = ri->lto_symbol_name_prefixes;
        result = foreach_peer_file(ri, lto_driver, true);
    }

    if (result) {
        init_result_params(&params);
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_LTO;
        params.severity = RESULT_OK;
        add_result(ri, &params);
    }

    return result;
}
