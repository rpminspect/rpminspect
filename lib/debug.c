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
 * @file debug.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2020
 * @brief Debugging utility functions.
 * @copyright LGPL-3.0-or-later
 */

#include <stdio.h>
#include <assert.h>
#include "rpminspect.h"

/**
 * @brief Set the global debugging mode.
 *
 * Pass true to enable debugging messages, false to disable.  Usually
 * used in a frontend program after reading in configuration files but
 * before collecting builds and running inspections.
 *
 * @param debug True to enable debugging messages, false to disable.
 */
void set_debug_mode(bool debug)
{
    debug_mode = debug;
    return;
}

/**
 * In debug mode, dump the current configuration settings that are in
 * effect for this run of rpminspect.  The configuration is displayed
 * on stderr in YAML structure.
 *
 * @param ri The main struct rpminspect for the program.
 */
void dump_cfg(const struct rpminspect *ri)
{
    int i = 0;
    string_entry_t *entry = NULL;
    ENTRY e;
    ENTRY *eptr;

    if (!debug_mode) {
        return;
    }

    assert(ri != NULL);

    fprintf(stderr, "Program Configuration\n==========\n");

    fprintf(stderr, "    ---\n");

    if (ri->workdir || ri->profiledir) {
        fprintf(stderr, "    common:\n");
        if (ri->workdir) {
            fprintf(stderr, "        workdir: %s\n", ri->workdir);
        }
        if (ri->profiledir) {
            fprintf(stderr, "        profiledir: %s\n", ri->profiledir);
        }
    }

    if (ri->kojihub || ri->kojiursine || ri->kojimbs) {
        fprintf(stderr, "    koji:\n");
        if (ri->kojihub) {
            fprintf(stderr, "        hub: %s\n", ri->kojihub);
        }
        if (ri->kojiursine) {
            fprintf(stderr, "        download_ursine: %s\n", ri->kojiursine);
        }
        if (ri->kojimbs) {
            fprintf(stderr, "        download_mbs: %s\n", ri->kojimbs);
        }
    }

    fprintf(stderr, "    vendor:\n");
    if (ri->vendor_data_dir) {
        fprintf(stderr, "        vendor_data_dir: %s\n", ri->vendor_data_dir);
    }
    if (ri->licensedb) {
        fprintf(stderr, "        licensedb: %s\n", ri->licensedb);
    }
    fprintf(stderr, "        favor_release: %s\n", (ri->favor_release == FAVOR_NONE) ? "none" : (ri->favor_release == FAVOR_OLDEST) ? "oldest" : (ri->favor_release == FAVOR_NEWEST) ? "newest" : "?");

    fprintf(stderr, "    inspections:\n");
    for (i = 0; inspections[i].flag != 0; i++) {
        fprintf(stderr, "        %s: %s\n", inspections[i].name, (ri->tests & inspections[i].flag) ? "on" : "off");
    }

    if (ri->product_keys && !TAILQ_EMPTY(ri->product_keys)) {
        fprintf(stderr, "    products:\n");
        TAILQ_FOREACH(entry, ri->product_keys, items) {
            e.key = entry->data;
            hsearch_r(e, FIND, &eptr, ri->products);

            if ((eptr != NULL) && (eptr->data != NULL)) {
                fprintf(stderr, "        - %s: %s\n", entry->data, (char *) eptr->data);
            }
        }
    }

    if (ri->ignores && !TAILQ_EMPTY(ri->ignores)) {
        fprintf(stderr, "    ignore:\n");
        TAILQ_FOREACH(entry, ri->ignores, items) {
            fprintf(stderr, "        - %s\n", entry->data);
        }
    }

    if (ri->security_path_prefix && !TAILQ_EMPTY(ri->security_path_prefix)) {
        fprintf(stderr, "    security_path_prefix:\n");
        TAILQ_FOREACH(entry, ri->security_path_prefix, items) {
            fprintf(stderr, "        - %s\n", entry->data);
        }
    }

    if (ri->badwords && !TAILQ_EMPTY(ri->badwords)) {
        fprintf(stderr, "    badwords:\n");
        TAILQ_FOREACH(entry, ri->badwords, items) {
            fprintf(stderr, "        - %s\n", entry->data);
        }
    }

    if (ri->vendor || (ri->buildhost_subdomain && !TAILQ_EMPTY(ri->buildhost_subdomain))) {
        fprintf(stderr, "    metadata:\n");
        if (ri->vendor) {
            fprintf(stderr, "        vendor: %s\n", ri->vendor);
        }
        if (ri->buildhost_subdomain && !TAILQ_EMPTY(ri->buildhost_subdomain)) {
            fprintf(stderr, "        buildhost_subdomain:\n");
            TAILQ_FOREACH(entry, ri->buildhost_subdomain, items) {
                fprintf(stderr, "            - %s\n", entry->data);
            }
        }
    }

    if (ri->elf_path_include_pattern || ri->elf_path_exclude_pattern || (ri->forbidden_ipv6_functions && !TAILQ_EMPTY(ri->forbidden_ipv6_functions))) {
        fprintf(stderr, "    elf:\n");
        if (ri->elf_path_include_pattern) {
            fprintf(stderr, "        include_path: %s\n", ri->elf_path_include_pattern);
        }
        if (ri->elf_path_exclude_pattern) {
            fprintf(stderr, "        exclude_path: %s\n", ri->elf_path_exclude_pattern);
        }
        if (ri->forbidden_ipv6_functions && !TAILQ_EMPTY(ri->forbidden_ipv6_functions)) {
            fprintf(stderr, "        forbidden_ipv6_functions:\n");
            TAILQ_FOREACH(entry, ri->forbidden_ipv6_functions, items) {
                fprintf(stderr, "            - %s\n", entry->data);
            }
        }
    }

    if (ri->manpage_path_include_pattern || ri->manpage_path_exclude_pattern) {
        fprintf(stderr, "    manpage:\n");
        if (ri->manpage_path_include_pattern) {
            fprintf(stderr, "        include_path: %s\n", ri->manpage_path_include_pattern);
        }
        if (ri->manpage_path_exclude_pattern) {
            fprintf(stderr, "        exclude_path: %s\n", ri->manpage_path_exclude_pattern);
        }
    }

    if (ri->xml_path_include_pattern || ri->xml_path_exclude_pattern) {
        fprintf(stderr, "    xml:\n");
        if (ri->xml_path_include_pattern) {
            fprintf(stderr, "        include_path: %s\n", ri->xml_path_include_pattern);
        }
        if (ri->xml_path_exclude_pattern) {
            fprintf(stderr, "        exclude_path: %s\n", ri->xml_path_exclude_pattern);
        }
    }

    if (ri->desktop_entry_files_dir) {
        fprintf(stderr, "    desktop:\n");
        fprintf(stderr, "        desktop_entry_files_dir: %s\n", ri->desktop_entry_files_dir);
    }

    if (ri->header_file_extensions && !TAILQ_EMPTY(ri->header_file_extensions)) {
        fprintf(stderr, "    changedfiles:\n");
        fprintf(stderr, "        header_file_extensions:\n");
        TAILQ_FOREACH(entry, ri->header_file_extensions, items) {
            fprintf(stderr, "            - %s\n", entry->data);
        }
    }

    if ((ri->forbidden_path_prefixes && !TAILQ_EMPTY(ri->forbidden_path_prefixes)) || (ri->forbidden_path_suffixes && !TAILQ_EMPTY(ri->forbidden_path_suffixes)) || (ri->forbidden_directories && !TAILQ_EMPTY(ri->forbidden_directories))) {
        fprintf(stderr, "    addedfiles:\n");
        if (ri->forbidden_path_prefixes && !TAILQ_EMPTY(ri->forbidden_path_prefixes)) {
            fprintf(stderr, "        forbidden_path_prefixes:\n");
            TAILQ_FOREACH(entry, ri->forbidden_path_prefixes, items) {
                fprintf(stderr, "            - %s\n", entry->data);
            }
        }
        if (ri->forbidden_path_suffixes && !TAILQ_EMPTY(ri->forbidden_path_suffixes)) {
            fprintf(stderr, "        forbidden_path_suffixes:\n");
            TAILQ_FOREACH(entry, ri->forbidden_path_suffixes, items) {
                fprintf(stderr, "            - %s\n", entry->data);
            }
        }
        if (ri->forbidden_directories && !TAILQ_EMPTY(ri->forbidden_directories)) {
            fprintf(stderr, "        forbidden_directories:\n");
            TAILQ_FOREACH(entry, ri->forbidden_directories, items) {
                fprintf(stderr, "            - %s\n", entry->data);
            }
        }
    }

    if ((ri->bin_paths && !TAILQ_EMPTY(ri->bin_paths)) || ri->bin_owner || ri->bin_group || (ri->forbidden_owners && !TAILQ_EMPTY(ri->forbidden_owners)) || (ri->forbidden_groups && !TAILQ_EMPTY(ri->forbidden_groups))) {
        fprintf(stderr, "    ownership:\n");
        if (ri->bin_paths && !TAILQ_EMPTY(ri->bin_paths)) {
            fprintf(stderr, "        bin_paths:\n");
            TAILQ_FOREACH(entry, ri->bin_paths, items) {
                fprintf(stderr, "            - %s\n", entry->data);
            }
        }
        if (ri->bin_owner) {
            fprintf(stderr, "        bin_owner: %s\n", ri->bin_owner);
        }
        if (ri->bin_group) {
            fprintf(stderr, "        bin_group: %s\n", ri->bin_group);
        }
        if (ri->forbidden_owners && !TAILQ_EMPTY(ri->forbidden_owners)) {
            fprintf(stderr, "        forbidden_owners:\n");
            TAILQ_FOREACH(entry, ri->forbidden_owners, items) {
                fprintf(stderr, "            - %s\n", entry->data);
            }
        }
        if (ri->forbidden_groups && !TAILQ_EMPTY(ri->forbidden_groups)) {
            fprintf(stderr, "        forbidden_groups:\n");
            TAILQ_FOREACH(entry, ri->forbidden_groups, items) {
                fprintf(stderr, "            - %s\n", entry->data);
            }
        }
    }

    if (ri->shells && !TAILQ_EMPTY(ri->shells)) {
        fprintf(stderr, "    shellsyntax:\n");
        fprintf(stderr, "        shells:\n");
        TAILQ_FOREACH(entry, ri->shells, items) {
            fprintf(stderr, "            - %s\n", entry->data);
        }
    }

    if (ri->size_threshold) {
        fprintf(stderr, "    filesize:\n");
        fprintf(stderr, "        size_threshold: %s\n", ri->size_threshold);
    }

    if (ri->lto_symbol_name_prefixes && !TAILQ_EMPTY(ri->lto_symbol_name_prefixes)) {
        fprintf(stderr, "    lto:\n");
        fprintf(stderr, "        lto_symbol_name_prefixes:\n");
        TAILQ_FOREACH(entry, ri->lto_symbol_name_prefixes, items) {
            fprintf(stderr, "            - %s\n", entry->data);
        }
    }

    fprintf(stderr, "    specname:\n");
    fprintf(stderr, "        match: %s\n", (ri->specmatch == MATCH_FULL) ? "full" : (ri->specmatch == MATCH_PREFIX) ? "prefix" : (ri->specmatch == MATCH_SUFFIX) ? "suffix" : "?");
    fprintf(stderr, "        primary: %s\n", (ri->specprimary == PRIMARY_NAME) ? "name" : (ri->specprimary == PRIMARY_FILENAME) ? "filename" : "?");

    if (ri->annocheck_keys && !TAILQ_EMPTY(ri->annocheck_keys)) {
        fprintf(stderr, "    annocheck:\n");
        TAILQ_FOREACH(entry, ri->annocheck_keys, items) {
            e.key = entry->data;
            hsearch_r(e, FIND, &eptr, ri->annocheck);

            if ((eptr != NULL) && (eptr->data != NULL)) {
                fprintf(stderr, "        - %s: %s\n", entry->data, (char *) eptr->data);
            }
        }
    }

    if (ri->jvm_keys && !TAILQ_EMPTY(ri->jvm_keys)) {
        fprintf(stderr, "    javabytecode:\n");
        TAILQ_FOREACH(entry, ri->jvm_keys, items) {
            e.key = entry->data;
            hsearch_r(e, FIND, &eptr, ri->jvm);

            if ((eptr != NULL) && (eptr->data != NULL)) {
                fprintf(stderr, "        - %s: %s\n", entry->data, (char *) eptr->data);
            }
        }
    }

    if (ri->pathmigration_keys && !TAILQ_EMPTY(ri->pathmigration_keys)) {
        fprintf(stderr, "    pathmigration:\n");
        TAILQ_FOREACH(entry, ri->pathmigration_keys, items) {
            e.key = entry->data;
            hsearch_r(e, FIND, &eptr, ri->pathmigration);

            if ((eptr != NULL) && (eptr->data != NULL)) {
                fprintf(stderr, "        - %s: %s\n", entry->data, (char *) eptr->data);
            }
        }
    }

    fprintf(stderr, "==========\n");

    return;
}
