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
 * Helper to add entries to abi argument tables.
 */
void add_abi_argument(struct hsearch_data *table, const char *arg, const char *path, const Header hdr)
{
    string_list_t *list = NULL;
    string_entry_t *entry = NULL;
    const char *arch = NULL;
    ENTRY e;
    ENTRY *eptr;

    assert(table != NULL);
    assert(arg != NULL);
    assert(path != NULL);
    assert(hdr != NULL);

    if (!usable_path(path)) {
        return;
    }

    /* prepare the new list entry */
    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "%s %s", arg, path);
    assert(entry->data != NULL);

    /* we have the dir, add it */
    arch = get_rpm_header_arch(hdr);
    e.key = (char *) arch;
    assert(e.key != NULL);
    e.data = NULL;
    eptr = NULL;
    hsearch_r(e, FIND, &eptr, table);

    if (eptr == NULL) {
        /* does not exist in the table, create it and add it */
        list = calloc(1, sizeof(*list));
        assert(list != NULL);
        TAILQ_INIT(list);
        TAILQ_INSERT_TAIL(list, entry, items);

        e.key = (char *) arch;
        e.data = list;
        eptr = NULL;

        if (!hsearch_r(e, ENTER, &eptr, table)) {
            err(RI_PROGRAM_ERROR, "hsearch_r()");
        }
    } else {
        /* list exists, add another entry */
        list = (string_list_t *) eptr->data;
        TAILQ_INSERT_TAIL(list, entry, items);
    }

    return;
}

/*
 * Count ABI entries in the named ABI compat level file.
 */
size_t count_abi_entries(const string_list_t *contents)
{
    size_t count = 0;
    string_entry_t *entry = NULL;

    if (list_len(contents) == 0) {
        return 0;
    }

    TAILQ_FOREACH(entry, contents, items) {
        /* skip blank lines, comments, and level definitions */
        if (*entry->data == '#' || *entry->data == '\n' || *entry->data == '\r') {
            continue;
        }

        /* trim line ending characters */
        entry->data[strcspn(entry->data, "\r\n")] = '\0';

        /* determine if we are reading a new level or not */
        if (strprefix(entry->data, "[") && strsuffix(entry->data, "]") && strcasestr(entry->data, "level-")) {
            continue;
        }

        count++;
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
    string_list_t *contents = NULL;
    string_entry_t *line = NULL;
    string_list_t *linekv = NULL;
    string_list_t *dsos = NULL;
    string_entry_t *pkg = NULL;
    string_entry_t *dso = NULL;
    string_entry_t *dsoval = NULL;
    int found_level = 0;
    size_t table_size = 1;
    bool skip_entries = false;
    abi_list_t *list = NULL;
    abi_entry_t *entry = NULL;
    abi_pkg_entry_t *pkgentry = NULL;
    string_entry_t *keyentry = NULL;
    bool foundkey = false;
    int r = 0;
    ENTRY e;
    ENTRY *eptr;
    uint64_t levels = 0;

    assert(vendor_data_dir != NULL);
    assert(product_release != NULL);

    /* read the ABI compat level file to read */
    xasprintf(&abifile, "%s/%s/%s", vendor_data_dir, ABI_DIR, product_release);
    assert(abifile != NULL);
    contents = read_file(abifile);
    free(abifile);

    if (contents == NULL) {
        /* this product release lacks an ABI compat level file */
        return NULL;
    }

    /* we need a line count of the file */
    table_size = count_abi_entries(contents);

    if (table_size == 0) {
        warn("count_abi_entries()");
        list_free(contents, free);
        return NULL;
    }

    /* read in the ABI compat levels */
    TAILQ_FOREACH(line, contents, items) {
        /* skip blank lines and comments */
        if (line->data == NULL || *line->data == '#' || *line->data == '\n' || *line->data == '\r' || !strcmp(line->data, "")) {
            continue;
        }

        /* determine if we are reading a new level or not */
        if (strprefix(line->data, "[") && strsuffix(line->data, "]") && strcasestr(line->data, "level-")) {
            /* new compat level section */

            /*
             * get the compat level we found and make sure we have
             * seen it before
             */
            if (sscanf(line->data + 7, "%d]", &found_level) == EOF) {
                warn("malformed ABI level identifier: %s", line->data);
                skip_entries = true;
            }

            if (levels & (((uint64_t) 1) << found_level)) {
                warn("ABI level %d already defined", found_level);
                skip_entries = true;
            }

            if (skip_entries) {
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
            linekv = strsplit(line->data, "=");

            if (list_len(linekv) != 2) {
                warn("malformed ABI level entry: %s", line->data);
                list_free(linekv, free);
                continue;
            }

            pkg = TAILQ_FIRST(linekv);
            dso = TAILQ_NEXT(pkg, items);

            /* try to find this package in the ABI compat level table */
            foundkey = false;

            if (entry->keys != NULL && !TAILQ_EMPTY(entry->keys)) {
                /* package name may already exist in the key list */
                TAILQ_FOREACH(keyentry, entry->keys, items) {
                    if (!strcmp(keyentry->data, pkg->data)) {
                        foundkey = true;
                        break;
                    }
                }

                /* package name exists, look for the hash table entry */
                if (foundkey && entry->pkgs) {
                    e.key = pkg->data;
                    hsearch_r(e, FIND, &eptr, entry->pkgs);

                    if (eptr != NULL && eptr->data != NULL) {
                        pkgentry = (abi_pkg_entry_t *) eptr->data;
                    }
                }
            } else {
                /* we lack a key list, so start one */
                entry->keys = calloc(1, sizeof(*entry->keys));
                assert(entry->keys != NULL);
                TAILQ_INIT(entry->keys);
            }

            /* split all the DSO values */
            dsos = strsplit(dso->data, ",\n\r");

            if (dsos == NULL) {
                warn("malformed DSO list: %s", dso->data);
                list_free(linekv, free);
                continue;
            }

            /* make sure we have the hash table entry */
            if (pkgentry == NULL) {
                pkgentry = calloc(1, sizeof(*pkgentry));
                assert(pkgentry != NULL);
            }

            /* collect all the DSO values */
            TAILQ_FOREACH(dsoval, dsos, items) {
                if (!strcasecmp(dsoval->data, "all-dsos")) {
                    pkgentry->all = true;
                } else {
                    if (pkgentry->dsos == NULL) {
                        pkgentry->dsos = calloc(1, sizeof(*(pkgentry->dsos)));
                        assert(pkgentry->dsos != NULL);
                        TAILQ_INIT(pkgentry->dsos);
                    }

                    keyentry = calloc(1, sizeof(*keyentry));
                    assert(keyentry != NULL);
                    keyentry->data = strdup(dsoval->data);
                    assert(keyentry->data != NULL);
                    TAILQ_INSERT_TAIL(pkgentry->dsos, keyentry, items);
                }
            }

            /* if this was a new package, add it */

            /* hash table keys */
            if (!foundkey && (pkgentry->all || pkgentry->dsos != NULL)) {
                keyentry = calloc(1, sizeof(*keyentry));
                assert(keyentry != NULL);
                keyentry->data = strdup(pkg->data);
                assert(keyentry->data != NULL);
                TAILQ_INSERT_TAIL(entry->keys, keyentry, items);

                /* ensure we have a hash table */
                if (entry->pkgs == NULL) {
                    entry->pkgs = calloc(1, sizeof(*(entry->pkgs)));
                    r = hcreate_r(table_size * 1.25, entry->pkgs);
                    assert(r != 0);
                }

                /* add this new hash table entry */
                e.key = keyentry->data;
                e.data = pkgentry;
                eptr = NULL;

                if (!hsearch_r(e, ENTER, &eptr, entry->pkgs)) {
                    warn("unable to add %s to the ABI compatibility level %d structure", pkg->data, entry->level);
                }
            }

            /* clean up */
            list_free(linekv, free);
            list_free(dsos, free);
        }
    }

    /* clean up */
    list_free(contents, free);

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

    while (!TAILQ_EMPTY(list)) {
        entry = TAILQ_FIRST(list);
        TAILQ_REMOVE(list, entry, items);

        TAILQ_FOREACH(key, entry->keys, items) {
            e.key = key->data;
            eptr = NULL;
            hsearch_r(e, FIND, &eptr, entry->pkgs);

            if ((eptr != NULL) && (eptr->data != NULL)) {
                pkgentry = (abi_pkg_entry_t *) eptr->data;
                list_free(pkgentry->dsos, free);
            }
        }

        list_free(entry->keys, free);
        hdestroy_r(entry->pkgs);
        free(entry->pkgs);
        free(entry);
    }

    free(list);
    return;
}

/*
 * Get any .abignore files that exist in SRPM files in the build.
 * These are passed to every invocation of abidiff(1) if they exist.
 */
string_list_t *get_abi_suppressions(const struct rpminspect *ri, const char *suppression_file)
{
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    string_list_t *list = NULL;
    string_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(suppression_file != NULL);

    TAILQ_FOREACH(peer, ri->peers, items) {
        if (!headerIsSource(peer->after_hdr)) {
            continue;
        }

        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            if (!strcmp(file->localpath, suppression_file)) {
                if (list == NULL) {
                    list = calloc(1, sizeof(*list));
                    TAILQ_INIT(list);
                }

                entry = calloc(1, sizeof(*entry));
                xasprintf(&entry->data, "%s %s", ABI_SUPPRESSIONS, file->fullpath);
                assert(entry->data != NULL);
                TAILQ_INSERT_TAIL(list, entry, items);
            }
        }
    }

    return list;
}

/*
 * Gather paths.  This goes in a hash table where the key is the arch
 * name and the value is a string_list_t of the debug_info_dir1/2 or
 * header_dir1/2 arguments to abidiff(1) or kmidiff(1).
 */
struct hsearch_data *get_abi_dir_arg(struct rpminspect *ri, const size_t size, const char *suffix, const char *arg, const char *path, const int type)
{
    int result = 0;
    rpmpeer_entry_t *peer = NULL;
    const char *name = NULL;
    char *tmp = NULL;
    char *root = NULL;
    struct hsearch_data *table = NULL;
    Header h;

    assert(ri != NULL);
    assert(size > 0);
    assert(arg != NULL);
    assert(path != NULL);

    /* initialize the table */
    table = calloc(1, sizeof(*table));
    assert(table != NULL);
    result = hcreate_r(size * 1.25, table);
    assert(result != 0);

    /* collect each path argument by arch */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (headerIsSource(peer->before_hdr) || headerIsSource(peer->after_hdr)) {
            continue;
        }

        if (type == BEFORE_BUILD && peer->before_files && !TAILQ_EMPTY(peer->before_files)) {
            root = peer->before_root;
            h = peer->before_hdr;
            name = headerGetString(h, RPMTAG_NAME);

        } else if (type == AFTER_BUILD && peer->after_files && !TAILQ_EMPTY(peer->after_files)) {
            root = peer->after_root;
            h = peer->after_hdr;
            name = headerGetString(h, RPMTAG_NAME);
        } else {
            continue;
        }

        if (suffix && strsuffix(name, suffix)) {
            xasprintf(&tmp, "%s%s", root, path);
        } else {
            xasprintf(&tmp, "%s%s", root, path);
        }

        add_abi_argument(table, arg, tmp, h);
        free(tmp);
    }

    return table;
}
