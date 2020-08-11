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

#include <unistd.h>
#include <assert.h>
#include <err.h>
#include <search.h>
#include "rpminspect.h"

/*
 * Count ABI entries in the named ABI compat level file.
 */
size_t count_abi_entries(const char *abifile)
{
    size_t count = 0;
    FILE *input = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread = 0;

    assert(abifile != NULL);
    input = fopen(abifile, "r");

    if (input == NULL) {
        return 0;
    }

    while ((nread = getline(&line, &len, input)) != -1) {
        /* skip blank lines, comments, and level definitions */
        if (*line == '#' || *line == '\n' || *line == '\r') {
            free(line);
            line = NULL;
            continue;
        }

        /* trim line ending characters */
        line[strcspn(line, "\r\n")] = '\0';

        /* determine if we are reading a new level or not */
        if (strprefix(line, "[") && strsuffix(line, "]") && strcasestr(line, "level-")) {
            free(line);
            line = NULL;
            continue;
        }

        count++;
        free(line);
        line = NULL;
    }

    if (fclose(input) == -1) {
        warn("fclose()");
    }

    return count;
}



/*
 * @brief Given a vendor data dir and product release string, look for
 * an ABI compatibility level file.  If found, read it in and return
 * the newly constructed abi_list_t.  Caller is responsible for
 * freeing the returned data.
 */
abi_list_t *read_abi(const char *vendor_data_dir, const char *product_release)
{
    char *abifile = NULL;
    FILE *input = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread = 0;
    int found_level = 0;
    size_t table_size = 1;
    bool skip_entries = true;
    abi_list_t *list = NULL;
    abi_entry_t *entry = NULL;
    abi_pkg_entry_t *pkgentry = NULL;
    string_entry_t *keyentry = NULL;
    char *pkg = NULL;
    char *dso = NULL;
    char *dsoval = NULL;
    bool foundkey = false;
    int r = 0;
    ENTRY e;
    ENTRY *eptr;
    uint64_t levels = 0;

    assert(vendor_data_dir != NULL);
    assert(product_release != NULL);

    /* build the ABI compat level file to read */
    xasprintf(&abifile, "%s/%s/%s", vendor_data_dir, ABI_DIR, product_release);
    assert(abifile != NULL);

    if (access(abifile, R_OK)) {
        /* this product release lacks an ABI compat level file */
        free(abifile);
        return NULL;
    }

    /* we need a line count of the file */
    table_size = count_abi_entries(abifile);

    if (table_size == 0) {
        warn("count_abi_entries()");
        free(abifile);
        return NULL;
    }

    /* open the ABI compat level file */
    if ((input = fopen(abifile, "r")) == NULL) {
        warn("fopen()");
        free(abifile);
        return NULL;
    }

    /* don't need this anymore */
    free(abifile);

    /* read in the ABI compat levels */
    while ((nread = getline(&line, &len, input)) != -1) {
        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r') {
            free(line);
            line = NULL;
            continue;
        }

        /* trim line ending characters */
        line[strcspn(line, "\r\n")] = '\0';

        /* determine if we are reading a new level or not */
        if (strprefix(line, "[") && strsuffix(line, "]") && strcasestr(line, "level-")) {
            /* new compat level section */

            /*
             * get the compat level we found and make sure we have
             * seen it before
             */
            if (sscanf(line + 7, "%d]", &found_level) == EOF) {
                warn("malformed ABI level identifier: %s", line);
                skip_entries = true;
            }

            if (levels & (((uint64_t) 1) << found_level)) {
                warn("ABI level %d already defined", found_level);
                skip_entries = true;
            }

            if (skip_entries) {
                free(line);
                line = NULL;
                continue;
            }

            /* init the list if we need to */
            if (list == NULL) {
                list = calloc(1, sizeof(*list));
                assert(list != NULL);
                TAILQ_INIT(list);
            }

            /* create the new level entry */
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->level = found_level;
            entry->pkgs = NULL;
            entry->keys = NULL;
            TAILQ_INSERT_TAIL(list, entry, items);
        } else {
            /* looking at an abi package entry line */

            /* split in to package and DSO list first */
            pkg = strsep(&line, "=");
            dso = strsep(&line, "=");

            if (pkg == NULL || dso == NULL || strsep(&line, "=") != NULL) {
                warn("malformed ABI level entry: %s", line);
                free(line);
                line = NULL;
                continue;
            }

            /* try to find this package in the ABI compat level table */
            foundkey = false;

            if (entry->keys) {
                /* package name may already exist in the key list */
                TAILQ_FOREACH(keyentry, entry->keys, items) {
                    if (!strcmp(keyentry->data, pkg)) {
                        foundkey = true;
                        break;
                    }
                }

                /* package name exists, look for the hash table entry */
                if (foundkey && entry->pkgs) {
                    e.key = pkg;
                    hsearch_r(e, FIND, &eptr, entry->pkgs);

                    if (eptr != NULL) {
                        pkgentry = (abi_pkg_entry_t *) eptr->data;
                    }
                }
            } else {
                /* we lack a key list, so start one */
                entry->keys = calloc(1, sizeof(*entry->keys));
                assert(entry->keys != NULL);
                TAILQ_INIT(entry->keys);
            }

            /* make sure we have the hash table entry */
            if (pkgentry == NULL) {
                pkgentry = calloc(1, sizeof(*pkgentry));
                assert(pkgentry != NULL);
            }

            /* collect all the DSO values */
            while ((dsoval = strsep(&dso, ",")) != NULL) {
                if (!strcasecmp(dsoval, "all-dsos")) {
                    pkgentry->all = true;
                } else {
                    if (pkgentry->dsos == NULL) {
                        pkgentry->dsos = calloc(1, sizeof(*pkgentry->dsos));
                        assert(pkgentry->dsos != NULL);
                        TAILQ_INIT(pkgentry->dsos);
                    }

                    keyentry = calloc(1, sizeof(*keyentry));
                    assert(keyentry != NULL);
                    keyentry->data = strdup(dsoval);
                    TAILQ_INSERT_TAIL(pkgentry->dsos, keyentry, items);
                }
            }

            /* if this was a new package, add it */
            if (!foundkey) {
                /* ensure we have a hash table */
                if (entry->pkgs == NULL) {
                    entry->pkgs = calloc(1, sizeof(*(entry->pkgs)));
                    r = hcreate_r(table_size * 1.25, entry->pkgs);
                    assert(r != 0);
                }

                /* add this new hash table entry */
                e.key = pkg;
                e.data = pkgentry;

                if (!hsearch_r(e, ENTER, &eptr, entry->pkgs)) {
                    warn("unable to add %s to the ABI compatibility level %d structure", pkg, entry->level);
                }

                /* hash table keys */
                keyentry = calloc(1, sizeof(*keyentry));
                assert(keyentry != NULL);
                keyentry->data = strdup(pkg);
                TAILQ_INSERT_TAIL(entry->keys, keyentry, items);
            }
        }

        /* clean up */
        free(line);
        line = NULL;
    }

    /* clean up */
    if (fclose(input) == -1) {
        warn("fclose()");
    }

    return list;
}

/*
 * Given an abi_list_t, free all the memory associated with it.
 */
void free_abi(abi_list_t *list)
{
    abi_entry_t *entry = NULL;
    string_entry_t *key = NULL;
    ENTRY e;
    ENTRY *eptr;
    abi_pkg_entry_t *pkgentry = NULL;

    if (list == NULL) {
        return;
    }

    TAILQ_FOREACH(entry, list, items) {
        if (entry->keys && entry->pkgs) {
            TAILQ_FOREACH(key, entry->keys, items) {
                e.key = key->data;
                hsearch_r(e, FIND, &eptr, entry->pkgs);

                if (eptr != NULL) {
                    pkgentry = (abi_pkg_entry_t *) eptr->data;
                    assert(pkgentry != NULL);
                    list_free(pkgentry->dsos, free);
                }
            }

            list_free(entry->keys, free);
            hdestroy_r(entry->pkgs);
            free(entry->pkgs);
        }
    }

    free(list);
    return;
}
