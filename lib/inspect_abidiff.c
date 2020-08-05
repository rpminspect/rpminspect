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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>

#ifdef _COMPAT_QUEUE
#include "compat/queue.h"
#else
#include <sys/queue.h>
#endif

#include "rpminspect.h"

/* Globals */
static string_list_t *firstargs = NULL;
static string_list_t *suppressions = NULL;
static struct hsearch_data *debug_info_dir1_table;
static struct hsearch_data *debug_info_dir2_table;
static struct hsearch_data *headers_dir1_table;
static struct hsearch_data *headers_dir2_table;

/*
 * Free one of the command line option tables.
 */
static void free_argv_table(struct rpminspect *ri, struct hsearch_data *table)
{
    ENTRY e;
    ENTRY *eptr;
    string_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(ri->arches != NULL);

    if (table == NULL) {
        return;
    }

    TAILQ_FOREACH(entry, ri->arches, items) {
        e.key = entry->data;
        hsearch_r(e, FIND, &eptr, table);

        if (eptr != NULL) {
            list_free(eptr->data, free);
        }
    }

    hdestroy_r(table);
    free(table);

    return;
}

/*
 * Get any .abignore files that exist in SRPM files in the build.
 * These are passed to every invocation of abidiff(1) if they exist.
 */
static string_list_t *get_suppressions(const struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    string_list_t *list = NULL;
    string_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(ri->suppression_file != NULL);

    TAILQ_FOREACH(peer, ri->peers, items) {
        if (!headerIsSource(peer->after_hdr)) {
            continue;
        }

        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            if (!strcmp(file->localpath, ri->suppression_file)) {
                if (list == NULL) {
                    list = calloc(1, sizeof(*list));
                    TAILQ_INIT(list);
                }

                entry = calloc(1, sizeof(*entry));
                xasprintf(&entry->data, "--suppressions %s", file->fullpath);
                assert(entry->data != NULL);
                TAILQ_INSERT_TAIL(list, entry, items);
            }
        }
    }

    return list;
}

/*
 * Checks to see if the given path is a readable directory.
 */
static bool usable_dir(const char *path)
{
    struct stat sb;

    if (path == NULL) {
        return false;
    }

    if (access(path, R_OK) == -1) {
        return false;
    }

    memset(&sb, 0, sizeof(sb));

    if (lstat(path, &sb) == -1) {
        warn("lstat()");
        return false;
    }

    if (!S_ISDIR(sb.st_mode)) {
        return false;
    }

    return true;
}

/*
 * Gather paths.  This goes in a hash table where the key is the arch
 * name and the value is a string_list_t of the debug_info_dir1/2 or
 * header_dir1/2 arguments to abidiff(1).
 */
static void get_dirs(struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;
    string_list_t *list = NULL;
    string_entry_t *entry = NULL;
    const char *name = NULL;
    const char *arch = NULL;
    char *tmp = NULL;
    size_t num_arches = 0;
    int result = 0;
    ENTRY e;
    ENTRY *eptr;

    assert(ri != NULL);
    assert(ri->arches != NULL);

    /* size of our architecture list */
    num_arches = list_len(ri->arches);

    /* initialize our hash tables */
    if (debug_info_dir1_table == NULL) {
        debug_info_dir1_table = calloc(1, sizeof(*debug_info_dir1_table));
        assert(debug_info_dir1_table != NULL);
        result = hcreate_r(num_arches * 1.25, debug_info_dir1_table);
        assert(result != 0);
    }

    if (debug_info_dir2_table == NULL) {
        debug_info_dir2_table = calloc(1, sizeof(*debug_info_dir2_table));
        assert(debug_info_dir2_table != NULL);
        result = hcreate_r(num_arches * 1.25, debug_info_dir2_table);
        assert(result != 0);
    }

    if (headers_dir1_table == NULL) {
        headers_dir1_table = calloc(1, sizeof(*headers_dir1_table));
        assert(headers_dir1_table != NULL);
        result = hcreate_r(num_arches * 1.25, headers_dir1_table);
        assert(result != 0);
    }

    if (headers_dir2_table == NULL) {
        headers_dir2_table = calloc(1, sizeof(*headers_dir2_table));
        assert(headers_dir2_table != NULL);
        result = hcreate_r(num_arches * 1.25, headers_dir2_table);
        assert(result != 0);
    }

    /* collect each path argument by arch */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (headerIsSource(peer->before_hdr) || headerIsSource(peer->after_hdr)) {
            continue;
        }

        /* paths from the before build go in the 'dir1' tables */
        if (peer->before_files && !TAILQ_EMPTY(peer->before_files)) {
            name = headerGetString(peer->before_hdr, RPMTAG_NAME);

            if (strsuffix(name, DEBUGINFO_SUFFIX)) {
                /* if this is a debuginfo package, find the debuginfo dir */
                xasprintf(&tmp, "%s%s", peer->before_root, ri->debuginfo_path);
                assert(tmp != NULL);

                if (!usable_dir(tmp)) {
                    free(tmp);
                    continue;
                }

                /* prepare the new list entry */
                entry = calloc(1, sizeof(*entry));
                assert(entry != NULL);
                xasprintf(&entry->data, "--debug-info-dir1 %s", tmp);
                assert(entry->data != NULL);
                free(tmp);

                /* we have the debug info dir, add it */
                arch = get_rpm_header_arch(peer->before_hdr);
                e.key = (char *) arch;
                hsearch_r(e, FIND, &eptr, debug_info_dir1_table);

                if (eptr == NULL) {
                    /* does not exist in the table, create it and add it */
                    list = calloc(1, sizeof(*list));
                    assert(list != NULL);
                    TAILQ_INIT(list);
                    TAILQ_INSERT_TAIL(list, entry, items);

                    e.key = (char *) arch;
                    e.data = list;

                    if (!hsearch_r(e, ENTER, &eptr, debug_info_dir1_table)) {
                        err(RI_PROGRAM_ERROR, "hsearch_r()");
                    }
                } else {
                    /* list exists, add another entry */
                    list = (string_list_t *) eptr->data;
                    TAILQ_INSERT_TAIL(list, entry, items);
                }
            } else {
                /* gather the header paths */
                xasprintf(&tmp, "%s%s", peer->before_root, ri->include_path);
                assert(tmp != NULL);

                if (!usable_dir(tmp)) {
                    free(tmp);
                    continue;
                }

                /* prepare the new list entry */
                entry = calloc(1, sizeof(*entry));
                assert(entry != NULL);
                xasprintf(&entry->data, "--headers-dir1 %s", tmp);
                assert(entry->data != NULL);
                free(tmp);

                /* we have the headers dir, add it */
                arch = get_rpm_header_arch(peer->before_hdr);
                e.key = (char *) arch;
                hsearch_r(e, FIND, &eptr, headers_dir1_table);

                if (eptr == NULL) {
                    /* does not exist in the table, create it and add it */
                    list = calloc(1, sizeof(*list));
                    assert(list != NULL);
                    TAILQ_INIT(list);
                    TAILQ_INSERT_TAIL(list, entry, items);

                    e.key = (char *) arch;
                    e.data = list;

                    if (!hsearch_r(e, ENTER, &eptr, headers_dir1_table)) {
                        err(RI_PROGRAM_ERROR, "hsearch_r()");
                    }
                } else {
                    /* list exists, add another entry */
                    list = (string_list_t *) eptr->data;
                    TAILQ_INSERT_TAIL(list, entry, items);
                }
            }
        }

        /* paths from the after build go in the 'dir2' tables */
        if (peer->after_files && !TAILQ_EMPTY(peer->after_files)) {
            name = headerGetString(peer->after_hdr, RPMTAG_NAME);

            if (strsuffix(name, DEBUGINFO_SUFFIX)) {
                /* if this is a debuginfo package, find the debuginfo dir */
                xasprintf(&tmp, "%s%s", peer->after_root, ri->debuginfo_path);
                assert(tmp != NULL);

                if (!usable_dir(tmp)) {
                    free(tmp);
                    continue;
                }

                /* prepare the new list entry */
                entry = calloc(1, sizeof(*entry));
                assert(entry != NULL);
                xasprintf(&entry->data, "--debug-info-dir2 %s", tmp);
                assert(entry->data != NULL);
                free(tmp);

                /* we have the debug info dir, add it */
                arch = get_rpm_header_arch(peer->after_hdr);
                e.key = (char *) arch;
                hsearch_r(e, FIND, &eptr, debug_info_dir2_table);

                if (eptr == NULL) {
                    /* does not exist in the table, create it and add it */
                    list = calloc(1, sizeof(*list));
                    assert(list != NULL);
                    TAILQ_INIT(list);
                    TAILQ_INSERT_TAIL(list, entry, items);

                    e.key = (char *) arch;
                    e.data = list;

                    if (!hsearch_r(e, ENTER, &eptr, debug_info_dir2_table)) {
                        err(RI_PROGRAM_ERROR, "hsearch_r()");
                    }
                } else {
                    /* list exists, add another entry */
                    list = (string_list_t *) eptr->data;
                    TAILQ_INSERT_TAIL(list, entry, items);
                }
            } else {
                /* gather the header paths */
                xasprintf(&tmp, "%s%s", peer->after_root, ri->include_path);
                assert(tmp != NULL);

                if (!usable_dir(tmp)) {
                    free(tmp);
                    continue;
                }

                /* prepare the new list entry */
                entry = calloc(1, sizeof(*entry));
                assert(entry != NULL);
                xasprintf(&entry->data, "--headers-dir2 %s", tmp);
                assert(entry->data != NULL);
                free(tmp);

                /* we have the headers dir, add it */
                arch = get_rpm_header_arch(peer->after_hdr);
                e.key = (char *) arch;
                hsearch_r(e, FIND, &eptr, headers_dir2_table);

                if (eptr == NULL) {
                    /* does not exist in the table, create it and add it */
                    list = calloc(1, sizeof(*list));
                    assert(list != NULL);
                    TAILQ_INIT(list);
                    TAILQ_INSERT_TAIL(list, entry, items);

                    e.key = (char *) arch;
                    e.data = list;

                    if (!hsearch_r(e, ENTER, &eptr, headers_dir2_table)) {
                        err(RI_PROGRAM_ERROR, "hsearch_r()");
                    }
                } else {
                    /* list exists, add another entry */
                    list = (string_list_t *) eptr->data;
                    TAILQ_INSERT_TAIL(list, entry, items);
                }
            }
        }
    }

    return;
}

static bool abidiff_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
//    bool rebase = false;
    string_list_t *argv = NULL;
    string_list_t *local_suppressions = NULL;
    string_list_t *local_d1 = NULL;
    string_list_t *local_d2 = NULL;
    string_list_t *arglist = NULL;
    string_entry_t *before = NULL;
    string_entry_t *after = NULL;
    const char *arch = NULL;
    ENTRY e;
    ENTRY *eptr;
    int exitcode;
    char *details = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);
    assert(firstargs != NULL);

    /* skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* skip anything without a peer */
    if (file->peer_file == NULL) {
        return true;
    }

    /* skip anything that is not an ELF file */
    if (!S_ISREG(file->st.st_mode) || !is_elf(file->fullpath)) {
        return true;
    }

    /* get the package architecture */
    arch = get_rpm_header_arch(file->rpm_header);

    /* build the abidiff command */
    argv = list_copy(firstargs);

    if (suppressions && !TAILQ_EMPTY(suppressions)) {
        local_suppressions = list_copy(suppressions);
        TAILQ_CONCAT(argv, local_suppressions, items);
    }

    e.key = (char *) arch;
    hsearch_r(e, FIND, &eptr, debug_info_dir1_table);

    if (eptr != NULL) {
        arglist = (string_list_t *) eptr->data;

        if (arglist && !TAILQ_EMPTY(arglist)) {
            local_d1 = list_copy(arglist);
            TAILQ_CONCAT(argv, local_d1, items);
        }
    }

    e.key = (char *) arch;
    hsearch_r(e, FIND, &eptr, debug_info_dir2_table);

    if (eptr != NULL) {
        arglist = (string_list_t *) eptr->data;

        if (arglist && !TAILQ_EMPTY(arglist)) {
            local_d2 = list_copy(arglist);
            TAILQ_CONCAT(argv, local_d2, items);
        }
    }

//    if (headers_dir1 && !TAILQ_EMPTY(headers_dir1)) {
//        TAILQ_CONCAT(argv, headers_dir1, items);
//    }

//    if (headers_dir2 && !TAILQ_EMPTY(headers_dir2)) {
//        TAILQ_CONCAT(argv, headers_dir2, items);
//    }

    before = calloc(1, sizeof(*before));
    assert(before != NULL);
    before->data = strdup(file->peer_file->fullpath);
    assert(before->data != NULL);
    TAILQ_INSERT_TAIL(argv, before, items);

    after = calloc(1, sizeof(*after));
    assert(after != NULL);
    after->data = strdup(file->fullpath);
    assert(after->data != NULL);
    TAILQ_INSERT_TAIL(argv, after, items);

    /* run abidiff */
    details = sl_run_cmd(&exitcode, argv);
    free(details);








    details = list_to_string(argv, " ");
    DEBUG_PRINT("|%s|\n", details);
    free(details);


/*
 * XXX
 * rough idea:
 * for each ELF file pair:
 *     build an abidiff command line (point to debug info and header files)
 *         any srpm with a .abignore file needs to be added as --suppressions on the command line (use full path to file)
 *     run abidiff
 *     report abidiff result:
 *         for failures, check against abi whitelist
 *         anything that is an ABI break in level 1 or 2 is a failure, every other break is info only if it's in the whitelist
 */


    /* determine if this is a rebase build */
//    rebase = is_rebase(ri);

    init_result_params(&params);
    params.header = HEADER_ABIDIFF;




    /* cleanup */
    list_free(argv, free);
    free(local_suppressions);
    free(local_d1);
    free(local_d2);

    return true;
}

/*
 * Main driver for the 'abidiff' inspection.
 */
bool inspect_abidiff(struct rpminspect *ri) {
    bool result = false;
    string_entry_t *entry = NULL;
    struct result_params params;

    assert(ri != NULL);

    /* if there's a .abignore file in the after SRPM, we need to use it */
    suppressions = get_suppressions(ri);

    /* get the debug info and header dirs */
    get_dirs(ri);

    /* build the list of first command line arguments */
    firstargs = calloc(1, sizeof(*firstargs));
    assert(firstargs != NULL);
    TAILQ_INIT(firstargs);

    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    entry->data = strdup(ABIDIFF_CMD);
    TAILQ_INSERT_TAIL(firstargs, entry, items);

    if (ri->abidiff_extra_args) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->data = strdup(ri->abidiff_extra_args);
        TAILQ_INSERT_TAIL(firstargs, entry, items);
    }

    /* run the main inspection */
    result = foreach_peer_file(ri, abidiff_driver, true);

    /* clean up */
    list_free(firstargs, free);
    list_free(suppressions, free);
    free_argv_table(ri, debug_info_dir1_table);
    free_argv_table(ri, debug_info_dir2_table);
    free_argv_table(ri, headers_dir1_table);
    free_argv_table(ri, headers_dir2_table);

    /* report the inspection results */
    init_result_params(&params);
    params.waiverauth = NOT_WAIVABLE;
    params.header = HEADER_ABIDIFF;

    if (result) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    }

    return result;
}
