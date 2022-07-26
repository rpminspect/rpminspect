/*
 * Copyright 2020 Red Hat, Inc.
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

#include <unistd.h>
#include <assert.h>
#include <err.h>
#include <ftw.h>
#include "uthash.h"
#include "rpminspect.h"

/* Global variables */
static const char *subdir = NULL;
static string_list_map_t **dest_table = NULL;
static Header rpmhdr;

/*
 * Helper to add entries to abi argument tables.
 */
static void add_abi_argument(string_list_map_t **table, const char *path, const Header hdr)
{
    string_entry_t *entry = NULL;
    string_list_map_t *hentry = NULL;
    const char *arch = NULL;

    assert(path != NULL);
    assert(hdr != NULL);

    if (!usable_path(path)) {
        return;
    }

    /* prepare the new list entry */
    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    entry->data = strdup(path);
    assert(entry->data != NULL);

    /* look up the entry in the hash table */
    arch = get_rpm_header_arch(hdr);
    HASH_FIND_STR(*table, arch, hentry);

    if (hentry == NULL) {
        /* does not exist in the table, create it and add it */
        hentry = calloc(1, sizeof(*hentry));
        assert(hentry != NULL);

        hentry->key = strdup(arch);

        hentry->value = calloc(1, sizeof(*hentry->value));
        assert(hentry->value != NULL);
        TAILQ_INIT(hentry->value);
        TAILQ_INSERT_TAIL(hentry->value, entry, items);

        HASH_ADD_KEYPTR(hh, *table, hentry->key, strlen(hentry->key), hentry);
    } else {
        /* list exists, add another entry */
        TAILQ_INSERT_TAIL(hentry->value, entry, items);
    }

    return;
}

/*
 * @brief Given a vendor data dir and product release string, look for
 * an ABI compatibility level file.  If found, read it in and return
 * the newly constructed abi_t.  Caller is responsible for freeing the
 * returned data.
 */
abi_t *read_abi(const char *vendor_data_dir, const char *product_release)
{
    char *abifile = NULL;
    abi_t *r = NULL;
    abi_t *entry = NULL;
    string_entry_t *dsoval = NULL;
    string_entry_t *dsoentry = NULL;
    string_list_t *contents = NULL;
    string_entry_t *line = NULL;
    string_list_t *linekv = NULL;
    string_list_t *dsos = NULL;
    string_entry_t *pkg = NULL;
    string_entry_t *dso = NULL;
    int found_level = 0;
    bool skip_entries = false;
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

    /* read in the ABI compat levels */
    TAILQ_FOREACH(line, contents, items) {
        /* skip blank lines and comments */
        if (line->data == NULL || *line->data == '#' || *line->data == '\n' || *line->data == '\r' || !strcmp(line->data, "")) {
            continue;
        }

        /* determine if we are reading a new level or not */
        if (strprefix(line->data, "[") && strsuffix(line->data, "]") && (strcasestr(line->data, "level-") || strcasestr(line->data, "level "))) {
            /* new compat level section */

            /*
             * get the compat level we found and make sure we have
             * seen it before
             */
            if (sscanf(line->data + 7, "%d]", &found_level) == EOF) {
                warn(_("malformed ABI level identifier: %s"), line->data);
                skip_entries = true;
            }

            if (levels & (((uint64_t) 1) << found_level)) {
                warn(_("ABI level %d already defined"), found_level);
                skip_entries = true;
            }

            if (skip_entries) {
                continue;
            }
        } else {
            /* looking at an abi package entry line */

            /* split in to package and DSO list first */
            linekv = strsplit(line->data, "=");

            if (list_len(linekv) != 2) {
                warn(_("malformed ABI level entry: %s"), line->data);
                list_free(linekv, free);
                continue;
            }

            pkg = TAILQ_FIRST(linekv);
            dso = TAILQ_NEXT(pkg, items);

            /* split all the DSO values */
            dsos = strsplit(dso->data, ",\n\r");

            if (dsos == NULL) {
                warn("malformed DSO list: %s", dso->data);
                list_free(linekv, free);
                continue;
            }

            /* try to find this package in the ABI compat level table */
            HASH_FIND_STR(r, pkg->data, entry);

            /* package is not found, add it to the table */
            if (entry == NULL) {
                entry = calloc(1, sizeof(*entry));
                assert(entry != NULL);

                entry->pkg = strdup(pkg->data);
                assert(entry->pkg != NULL);

                entry->level = found_level;
                entry->all = false;
                entry->dsos = NULL;

                HASH_ADD_KEYPTR(hh, r, entry->pkg, strlen(entry->pkg), entry);
            }

            /* collect all the DSO values */
            TAILQ_FOREACH(dsoval, dsos, items) {
                if (!strcasecmp(dsoval->data, "all-dsos")) {
                    /* flag "all DSOs" as part of this ABI level */
                    entry->all = true;
                } else {
                    /* initialize the dsos list */
                    if (entry->dsos == NULL) {
                        entry->dsos = calloc(1, sizeof(*entry->dsos));
                        assert(entry->dsos != NULL);
                        TAILQ_INIT(entry->dsos);
                    }

                    dsoentry = calloc(1, sizeof(*dsoentry));
                    assert(dsoentry != NULL);
                    dsoentry->data = strdup(dsoval->data);
                    assert(dsoentry->data != NULL);
                    TAILQ_INSERT_TAIL(entry->dsos, dsoentry, items);
                }
            }

            /* clean up */
            list_free(linekv, free);
            list_free(dsos, free);
            entry = NULL;          /* for the next loop iteration */
        }
    }

    /* clean up */
    list_free(contents, free);

    return r;
}

/*
 * Given an abi_t, free all the memory associated with it.
 */
void free_abi(abi_t *table)
{
    abi_t *entry = NULL;
    abi_t *tmp_entry = NULL;

    if (table == NULL) {
        return;
    }

    HASH_ITER(hh, table, entry, tmp_entry) {
        HASH_DEL(table, entry);
        free(entry->pkg);
        list_free(entry->dsos, free);
    }

    free(table);
    return;
}

/*
 * Get any .abignore files that exist in SRPM files in the build.
 * These are passed to every invocation of abidiff(1) if they exist.
 */
string_list_t *get_abidiff_suppressions(const struct rpminspect *ri, const char *suppression_file)
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

/* nftw() callback for get_abidiff_dir_arg() below */
static int have_subdir(const char *fpath, __attribute__((unused)) const struct stat *sb, int tflag, __attribute__((unused)) struct FTW *ftwbuf)
{
    assert(fpath != NULL);
    assert(subdir != NULL);
    assert(rpmhdr != NULL);

    if ((tflag == FTW_D) && strsuffix(fpath, subdir)) {
        /* this path is a directory with the subdir in it */
        add_abi_argument(dest_table, fpath, rpmhdr);
    }

    return 0;
}

/*
 * Gather paths.  This goes in a hash table where the key is the arch
 * name and the value is a string_list_t of the debug_info_dir1/2 or
 * headers_dir1/2 arguments to abidiff(1) or kmidiff(1).
 */
string_list_map_t *get_abidiff_dir_arg(struct rpminspect *ri, const size_t size, const char *suffix, const char *path, const int type)
{
    rpmpeer_entry_t *peer = NULL;
    const char *name = NULL;
    char *tmp = NULL;
    char *root = NULL;
    string_list_map_t *table = NULL;
    Header h;
    int flags = FTW_MOUNT | FTW_PHYS;
    struct stat sb;

    assert(ri != NULL);
    assert(size > 0);
    assert(path != NULL);

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

        if ((suffix && strsuffix(name, suffix)) || (suffix == NULL)) {
            tmp = joinpath(root, path, NULL);
            assert(tmp != NULL);
        } else {
            continue;
        }

        if ((stat(tmp, &sb) == 0) && S_ISDIR(sb.st_mode)) {
            /* the simple case is that this path just exists */
            add_abi_argument(&table, tmp, h);
        } else {
            /*
             * the more complicated case involves looking for this
             * path string in all subdirectories in the tree
             */
            subdir = path;
            dest_table = &table;
            rpmhdr = h;

            if (nftw(root, have_subdir, FOPEN_MAX, flags) != 0) {
                warn("nftw");
            }
        }

        free(tmp);
    }

    return table;
}

/*
 * Try to find the debug subdirectory containing the debuginfo for the
 * file in question.
 */
char *add_abidiff_arg(char *cmd, string_list_map_t *table, const char *arch, const char *arg)
{
    string_list_map_t *hentry = NULL;
    string_entry_t *entry = NULL;

    if (table == NULL || arch == NULL) {
        return cmd;
    }

    HASH_FIND_STR(table, arch, hentry);

    if (hentry != NULL && hentry->value && !TAILQ_EMPTY(hentry->value)) {
        TAILQ_FOREACH(entry, hentry->value, items) {
            cmd = strappend(cmd, " ", arg, " ", entry->data, NULL);
        }
    }

    return cmd;
}
