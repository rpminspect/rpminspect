/*
 * Copyright © 2020 Red Hat, Inc.
 * Author(s): David Cantrell <dcantrell@redhat.com>
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
 * @date 2020-2021
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
    string_map_t *hentry = NULL;
    string_map_t *tmp_hentry = NULL;

    assert(ri != NULL);

    printf("# rpminspect configuration\n\n---\n");

    if (ri->workdir || ri->profiledir) {
        printf("common:\n");

        if (ri->workdir) {
            printf("    workdir: %s\n", ri->workdir);
        }

        if (ri->profiledir) {
            printf("    profiledir: %s\n", ri->profiledir);
        }
    }

    if (ri->kojihub || ri->kojiursine || ri->kojimbs) {
        printf("koji:\n");

        if (ri->kojihub) {
            printf("    hub: %s\n", ri->kojihub);
        }

        if (ri->kojiursine) {
            printf("    download_ursine: %s\n", ri->kojiursine);
        }

        if (ri->kojimbs) {
            printf("    download_mbs: %s\n", ri->kojimbs);
        }
    }

    printf("commands:\n");

    if (ri->commands.diff) {
        printf("    diff: %s\n", ri->commands.diff);
    }

    if (ri->commands.diffstat) {
        printf("    diffstat: %s\n", ri->commands.diffstat);
    }

    if (ri->commands.msgunfmt) {
        printf("    msgunfmt: %s\n", ri->commands.msgunfmt);
    }

    if (ri->commands.desktop_file_validate) {
        printf("    desktop-file-validate: %s\n", ri->commands.desktop_file_validate);
    }

    if (ri->commands.annocheck) {
        printf("    annocheck: %s\n", ri->commands.annocheck);
    }

    if (ri->commands.abidiff) {
        printf("    abidiff: %s\n", ri->commands.abidiff);
    }

    if (ri->commands.kmidiff) {
        printf("    kmidiff: %s\n", ri->commands.kmidiff);
    }

    printf("vendor:\n");

    if (ri->vendor_data_dir) {
        printf("    vendor_data_dir: %s\n", ri->vendor_data_dir);
    }

    if (ri->licensedb) {
        printf("    licensedb: %s\n", ri->licensedb);
    }

    printf("    favor_release: %s\n", (ri->favor_release == FAVOR_NONE) ? "none" : (ri->favor_release == FAVOR_OLDEST) ? "oldest" : (ri->favor_release == FAVOR_NEWEST) ? "newest" : "?");

    printf("inspections:\n");

    for (i = 0; inspections[i].name != NULL; i++) {
        printf("    %s: %s\n", inspections[i].name, (ri->tests & inspections[i].flag) ? "on" : "off");
    }

    if (ri->products) {
        printf("products:\n");

        HASH_ITER(hh, ri->products, hentry, tmp_hentry) {
            printf("    - %s: %s\n", hentry->key, hentry->value);
        }
    }

    if (ri->ignores && !TAILQ_EMPTY(ri->ignores)) {
        printf("ignore:\n");

        TAILQ_FOREACH(entry, ri->ignores, items) {
            printf("    - %s\n", entry->data);
        }
    }

    if (ri->security_path_prefix && !TAILQ_EMPTY(ri->security_path_prefix)) {
        printf("security_path_prefix:\n");

        TAILQ_FOREACH(entry, ri->security_path_prefix, items) {
            printf("    - %s\n", entry->data);
        }
    }

    if (ri->badwords && !TAILQ_EMPTY(ri->badwords)) {
        printf("badwords:\n");

        TAILQ_FOREACH(entry, ri->badwords, items) {
            printf("    - %s\n", entry->data);
        }
    }

    if (ri->vendor || (ri->buildhost_subdomain && !TAILQ_EMPTY(ri->buildhost_subdomain))) {
        printf("metadata:\n");

        if (ri->vendor) {
            printf("    vendor: %s\n", ri->vendor);
        }

        if (ri->buildhost_subdomain && !TAILQ_EMPTY(ri->buildhost_subdomain)) {
            printf("    buildhost_subdomain:\n");

            TAILQ_FOREACH(entry, ri->buildhost_subdomain, items) {
                printf("        - %s\n", entry->data);
            }
        }
    }

    if (ri->elf_path_include_pattern || ri->elf_path_exclude_pattern) {
        printf("elf:\n");

        if (ri->elf_path_include_pattern) {
            printf("    include_path: %s\n", ri->elf_path_include_pattern);
        }

        if (ri->elf_path_exclude_pattern) {
            printf("    exclude_path: %s\n", ri->elf_path_exclude_pattern);
        }
    }

    if (ri->manpage_path_include_pattern || ri->manpage_path_exclude_pattern) {
        printf("manpage:\n");

        if (ri->manpage_path_include_pattern) {
            printf("    include_path: %s\n", ri->manpage_path_include_pattern);
        }

        if (ri->manpage_path_exclude_pattern) {
            printf("    exclude_path: %s\n", ri->manpage_path_exclude_pattern);
        }
    }

    if (ri->xml_path_include_pattern || ri->xml_path_exclude_pattern) {
        printf("xml:\n");

        if (ri->xml_path_include_pattern) {
            printf("    include_path: %s\n", ri->xml_path_include_pattern);
        }

        if (ri->xml_path_exclude_pattern) {
            printf("    exclude_path: %s\n", ri->xml_path_exclude_pattern);
        }
    }

    if (ri->desktop_entry_files_dir) {
        printf("desktop:\n    desktop_entry_files_dir: %s\n", ri->desktop_entry_files_dir);
    }

    if (ri->header_file_extensions && !TAILQ_EMPTY(ri->header_file_extensions)) {
        printf("changedfiles:\n    header_file_extensions:\n");

        TAILQ_FOREACH(entry, ri->header_file_extensions, items) {
            printf("        - %s\n", entry->data);
        }
    }

    if ((ri->forbidden_path_prefixes && !TAILQ_EMPTY(ri->forbidden_path_prefixes)) || (ri->forbidden_path_suffixes && !TAILQ_EMPTY(ri->forbidden_path_suffixes)) || (ri->forbidden_directories && !TAILQ_EMPTY(ri->forbidden_directories))) {
        printf("addedfiles:\n");

        if (ri->forbidden_path_prefixes && !TAILQ_EMPTY(ri->forbidden_path_prefixes)) {
            printf("    forbidden_path_prefixes:\n");

            TAILQ_FOREACH(entry, ri->forbidden_path_prefixes, items) {
                printf("        - %s\n", entry->data);
            }
        }

        if (ri->forbidden_path_suffixes && !TAILQ_EMPTY(ri->forbidden_path_suffixes)) {
            printf("    forbidden_path_suffixes:\n");

            TAILQ_FOREACH(entry, ri->forbidden_path_suffixes, items) {
                printf("        - %s\n", entry->data);
            }
        }

        if (ri->forbidden_directories && !TAILQ_EMPTY(ri->forbidden_directories)) {
            printf("    forbidden_directories:\n");

            TAILQ_FOREACH(entry, ri->forbidden_directories, items) {
                printf("        - %s\n", entry->data);
            }
        }
    }

    if ((ri->bin_paths && !TAILQ_EMPTY(ri->bin_paths)) || ri->bin_owner || ri->bin_group || (ri->forbidden_owners && !TAILQ_EMPTY(ri->forbidden_owners)) || (ri->forbidden_groups && !TAILQ_EMPTY(ri->forbidden_groups))) {
        printf("ownership:\n");

        if (ri->bin_paths && !TAILQ_EMPTY(ri->bin_paths)) {
            printf("    bin_paths:\n");

            TAILQ_FOREACH(entry, ri->bin_paths, items) {
                printf("        - %s\n", entry->data);
            }
        }

        if (ri->bin_owner) {
            printf("    bin_owner: %s\n", ri->bin_owner);
        }

        if (ri->bin_group) {
            printf("    bin_group: %s\n", ri->bin_group);
        }

        if (ri->forbidden_owners && !TAILQ_EMPTY(ri->forbidden_owners)) {
            printf("    forbidden_owners:\n");

            TAILQ_FOREACH(entry, ri->forbidden_owners, items) {
                printf("        - %s\n", entry->data);
            }
        }

        if (ri->forbidden_groups && !TAILQ_EMPTY(ri->forbidden_groups)) {
            printf("    forbidden_groups:\n");

            TAILQ_FOREACH(entry, ri->forbidden_groups, items) {
                printf("        - %s\n", entry->data);
            }
        }
    }

    if (ri->shells && !TAILQ_EMPTY(ri->shells)) {
        printf("shellsyntax:\n    shells:\n");

        TAILQ_FOREACH(entry, ri->shells, items) {
            printf("        - %s\n", entry->data);
        }
    }

    if (ri->size_threshold) {
        printf("filesize:\n    size_threshold: ");

        if (ri->size_threshold == -1) {
            printf("info");
        } else {
            printf("%ld", ri->size_threshold);
        }

        printf("\n");
    }

    if (ri->lto_symbol_name_prefixes && !TAILQ_EMPTY(ri->lto_symbol_name_prefixes)) {
        printf("lto:\n    lto_symbol_name_prefixes:\n");

        TAILQ_FOREACH(entry, ri->lto_symbol_name_prefixes, items) {
            printf("        - %s\n", entry->data);
        }
    }

    printf("specname:\n");
    printf("    match: %s\n", (ri->specmatch == MATCH_FULL) ? "full" : (ri->specmatch == MATCH_PREFIX) ? "prefix" : (ri->specmatch == MATCH_SUFFIX) ? "suffix" : "?");
    printf("    primary: %s\n", (ri->specprimary == PRIMARY_NAME) ? "name" : (ri->specprimary == PRIMARY_FILENAME) ? "filename" : "?");

    if (ri->annocheck) {
        printf("annocheck:\n");

        HASH_ITER(hh, ri->annocheck, hentry, tmp_hentry) {
            printf("    - %s: %s\n", hentry->key, hentry->value);
        }
    }

    if (ri->jvm) {
        printf("javabytecode:\n");

        HASH_ITER(hh, ri->jvm, hentry, tmp_hentry) {
            printf("    - %s: %s\n", hentry->key, hentry->value);
        }
    }

    if (ri->pathmigration || (ri->pathmigration_excluded_paths && !TAILQ_EMPTY(ri->pathmigration_excluded_paths))) {
        printf("pathmigration:\n");

        if (ri->pathmigration) {
            printf("    migrated_paths:\n");

            HASH_ITER(hh, ri->pathmigration, hentry, tmp_hentry) {
                printf("        - %s: %s\n", hentry->key, hentry->value);
            }
        }

        if (ri->pathmigration_excluded_paths && !TAILQ_EMPTY(ri->pathmigration_excluded_paths)) {
            printf("    excluded_paths:\n");

            TAILQ_FOREACH(entry, ri->pathmigration_excluded_paths, items) {
                printf("        - %s\n", entry->data);
            }
        }
    }

    if (ri->forbidden_paths && !TAILQ_EMPTY(ri->forbidden_paths)) {
        printf("files:\n    forbidden_paths:\n");

        TAILQ_FOREACH(entry, ri->forbidden_paths, items) {
            printf("        - %s\n", entry->data);
        }
    }

    printf("abidiff:\n");

    if (ri->abidiff_suppression_file) {
        printf("    suppression_file: %s\n", ri->abidiff_suppression_file);
    }

    if (ri->abidiff_debuginfo_path) {
        printf("    debuginfo_path: %s\n", ri->abidiff_debuginfo_path);
    }

    if (ri->abidiff_include_path) {
        printf("    include_path: %s\n", ri->abidiff_include_path);
    }

    if (ri->abidiff_extra_args) {
        printf("    extra_args: %s\n", ri->abidiff_extra_args);
    }

    printf("    security_level_threshold: %ld\n", ri->abi_security_threshold);

    if (ri->kmidiff_suppression_file || ri->kmidiff_debuginfo_path || ri->kmidiff_extra_args || (ri->kernel_filenames && !TAILQ_EMPTY(ri->kernel_filenames)) || ri->kabi_dir || ri->kabi_filename) {
        printf("kmidiff:\n");

        if (ri->kmidiff_suppression_file) {
            printf("    suppression_file: %s\n", ri->kmidiff_suppression_file);
        }

        if (ri->kmidiff_debuginfo_path) {
            printf("    debuginfo_path: %s\n", ri->kmidiff_debuginfo_path);
        }

        if (ri->kmidiff_extra_args) {
            printf("    extra_args: %s\n", ri->kmidiff_extra_args);
        }

        if (ri->kernel_filenames && !TAILQ_EMPTY(ri->kernel_filenames)) {
            printf("    kernel_filenames:\n");

            TAILQ_FOREACH(entry, ri->kernel_filenames, items) {
                printf("        - %s\n", entry->data);
            }
        }

        if (ri->kabi_dir) {
            printf("    kabi_dir: %s\n", ri->kabi_dir);
        }

        if (ri->kabi_filename) {
            printf("    kabi_filename: %s\n", ri->kabi_filename);
        }
    }

    printf("patches:\n");

    if (ri->patch_ignore_list && !TAILQ_EMPTY(ri->patch_ignore_list)) {
        printf("    ignore_list:\n");

        TAILQ_FOREACH(entry, ri->patch_ignore_list, items) {
            printf("        - %s\n", entry->data);
        }
    }

    printf("    file_count_threshold: %ld\n", ri->patch_file_threshold);
    printf("    line_count_threshold: %ld\n", ri->patch_line_threshold);

    if (ri->bad_functions && !TAILQ_EMPTY(ri->bad_functions)) {
        printf("badfuncs:\n");

        TAILQ_FOREACH(entry, ri->bad_functions, items) {
            printf("    - %s\n", entry->data);
        }
    }

    if ((ri->runpath_allowed_paths && !TAILQ_EMPTY(ri->runpath_allowed_paths)) ||
        (ri->runpath_allowed_origin_paths && !TAILQ_EMPTY(ri->runpath_allowed_origin_paths)) ||
        (ri->runpath_origin_prefix_trim && !TAILQ_EMPTY(ri->runpath_origin_prefix_trim))) {
        printf("runpath:\n");

        if (ri->runpath_allowed_paths && !TAILQ_EMPTY(ri->runpath_allowed_paths)) {
            printf("    allowed_paths:\n");

            TAILQ_FOREACH(entry, ri->runpath_allowed_paths, items) {
                printf("        - %s\n", entry->data);
            }
        }

        if (ri->runpath_allowed_origin_paths && !TAILQ_EMPTY(ri->runpath_allowed_origin_paths)) {
            printf("    allowed_origin_paths:\n");

            TAILQ_FOREACH(entry, ri->runpath_allowed_origin_paths, items) {
                printf("        - %s\n", entry->data);
            }
        }

        if (ri->runpath_origin_prefix_trim && !TAILQ_EMPTY(ri->runpath_origin_prefix_trim)) {
            printf("    origin_prefix_trim:\n");

            TAILQ_FOREACH(entry, ri->runpath_origin_prefix_trim, items) {
                printf("        - %s\n", entry->data);
            }
        }
    }

    printf("\n\n");
    return;
}
