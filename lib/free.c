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

#include <regex.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <search.h>
#include "rpminspect.h"

void free_regex(regex_t *regex)
{
    if (regex == NULL) {
        return;
    }

    regfree(regex);
    free(regex);
}

void free_mapping(struct hsearch_data *table, string_list_t *keys)
{
    ENTRY e;
    ENTRY *eptr;
    string_entry_t *entry = NULL;

    if (table != NULL && keys != NULL) {
        /* look up each key and free the memory for the value */
        TAILQ_FOREACH(entry, keys, items) {
            e.key = entry->data;
            hsearch_r(e, FIND, &eptr, table);

            if (eptr != NULL) {
                free(eptr->data);
            }
        }

        /* destroy the hash table */
        hdestroy_r(table);
        free(table);

        /* destroy the list of keys */
        list_free(keys, free);
    }

    return;
}

/*
 * Free a struct rpminspect.  Called by applications using
 * librpminspect before they exit.
 */
void free_rpminspect(struct rpminspect *ri) {
    stat_whitelist_entry_t *swlentry = NULL;
    caps_whitelist_entry_t *cwlentry = NULL;
    caps_filelist_entry_t *cflentry = NULL;
    header_cache_entry_t *hentry = NULL;

    if (ri == NULL) {
        return;
    }

    free(ri->cfgfile);
    free(ri->workdir);
    free(ri->kojihub);
    free(ri->kojiursine);
    free(ri->kojimbs);
    free(ri->worksubdir);

    free(ri->vendor_data_dir);
    free(ri->licensedb);

    if (ri->stat_whitelist) {
        while (!TAILQ_EMPTY(ri->stat_whitelist)) {
            swlentry = TAILQ_FIRST(ri->stat_whitelist);
            TAILQ_REMOVE(ri->stat_whitelist, swlentry, items);

            free(swlentry->owner);
            free(swlentry->group);
            free(swlentry->filename);

            free(swlentry);
        }

        free(ri->stat_whitelist);
    }

    if (ri->caps_whitelist) {
        while (!TAILQ_EMPTY(ri->caps_whitelist)) {
            cwlentry = TAILQ_FIRST(ri->caps_whitelist);
            TAILQ_REMOVE(ri->caps_whitelist, cwlentry, items);

            free(cwlentry->pkg);

            if (cwlentry->files) {
                while (!TAILQ_EMPTY(cwlentry->files)) {
                    cflentry = TAILQ_FIRST(cwlentry->files);
                    TAILQ_REMOVE(cwlentry->files, cflentry, items);

                    free(cflentry->path);
                    free(cflentry->caps);

                    free(cflentry);
                }
            }
        }

        free(ri->caps_whitelist);
    }

    list_free(ri->badwords, free);

    free_regex(ri->elf_path_include);
    free_regex(ri->elf_path_exclude);
    free_regex(ri->manpage_path_include);
    free_regex(ri->manpage_path_exclude);
    free_regex(ri->xml_path_include);
    free_regex(ri->xml_path_exclude);

    free(ri->elf_path_include_pattern);
    free(ri->elf_path_exclude_pattern);
    free(ri->manpage_path_include_pattern);
    free(ri->manpage_path_exclude_pattern);
    free(ri->xml_path_include_pattern);
    free(ri->xml_path_exclude_pattern);

    free(ri->desktop_entry_files_dir);
    free(ri->vendor);
    list_free(ri->buildhost_subdomain, free);
    list_free(ri->security_path_prefix, free);
    list_free(ri->header_file_extensions, free);
    list_free(ri->forbidden_path_prefixes, free);
    list_free(ri->forbidden_path_suffixes, free);
    list_free(ri->forbidden_directories, free);
    free(ri->before);
    free(ri->after);
    free(ri->product_release);
    list_free(ri->arches, free);
    list_free(ri->bin_paths, free);
    free(ri->bin_owner);
    free(ri->bin_group);
    list_free(ri->forbidden_owners, free);
    list_free(ri->forbidden_groups, free);
    list_free(ri->shells, free);
    free(ri->size_threshold);
    free_mapping(ri->jvm, ri->jvm_keys);
    free_mapping(ri->annocheck, ri->annocheck_keys);
    free_mapping(ri->pathmigration, ri->pathmigration_keys);
    free_mapping(ri->products, ri->product_keys);
    list_free(ri->ignores, free);
    list_free(ri->lto_symbol_name_prefixes, free);

    free_rpmpeer(ri->peers);

    if (ri->header_cache != NULL) {
        while (!TAILQ_EMPTY(ri->header_cache)) {
            hentry = TAILQ_FIRST(ri->header_cache);
            TAILQ_REMOVE(ri->header_cache, hentry, items);
            free(hentry->pkg);
            headerFree(hentry->hdr);
            free(hentry);
        }

        free(ri->header_cache);
    }

    free(ri->before_rel);
    free(ri->after_rel);

    free_results(ri->results);

    return;
}
