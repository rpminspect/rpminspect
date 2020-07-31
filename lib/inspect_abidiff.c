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
static string_list_t *suppressions = NULL;
static string_list_t *debug_info_dir1 = NULL;
static string_list_t *debug_info_dir2 = NULL;
static string_list_t *headers_dir1 = NULL;
static string_list_t *headers_dir2 = NULL;

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
                entry->data = strdup(file->fullpath);
                DEBUG_PRINT("suppression option: |%s|\n", entry->data);
                TAILQ_INSERT_TAIL(list, entry, items);
            }
        }
    }

    return list;
}

static void get_debuginfo_dirs(const struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    string_entry_t *entry = NULL;
    const char *name = NULL;
    char *tmp = NULL;
    struct stat sb;

    assert(ri != NULL);

    TAILQ_FOREACH(peer, ri->peers, items) {
        if (headerIsSource(peer->before_hdr) || headerIsSource(peer->after_hdr)) {
            continue;
        }

        free(tmp);

        /* debuginfo dirs from the before build go in debug_info_dir1 */
        if (peer->before_files && !TAILQ_EMPTY(peer->before_files)) {
            name = headerGetString(peer->before_hdr, RPMTAG_NAME);

            if (strsuffix(name, DEBUGINFO_SUFFIX)) {
                xasprintf(&tmp, "%s%s", peer->before_root, ri->debuginfo_path);
                assert(tmp != NULL);
                memset(&sb, 0, sizeof(sb));

                if (lstat(tmp, &sb) == -1) {
                    warn("lstat()");
                    continue;
                }

                if (S_ISDIR(sb.st_mode)) {
                    if (debug_info_dir1 == NULL) {
                        debug_info_dir1 = calloc(1, sizeof(*debug_info_dir1));
                        TAILQ_INIT(debug_info_dir1);
                    }

                    entry = calloc(1, sizeof(*entry));
                    entry->data = strdup(tmp);
                    TAILQ_INSERT_TAIL(debug_info_dir1, entry, items);
                }

                free(tmp);
            }
        }

        free(tmp);

        /* debuginfo dirs from the after build go in debug_info_dir2 */
        if (peer->after_files && !TAILQ_EMPTY(peer->after_files)) {
            name = headerGetString(peer->after_hdr, RPMTAG_NAME);

            if (strsuffix(name, DEBUGINFO_SUFFIX)) {
                xasprintf(&tmp, "%s%s", peer->after_root, ri->debuginfo_path);
                assert(tmp != NULL);
                memset(&sb, 0, sizeof(sb));

                if (lstat(tmp, &sb) == -1) {
                    warn("lstat()");
                    continue;
                }

                if (S_ISDIR(sb.st_mode)) {
                    if (debug_info_dir2 == NULL) {
                        debug_info_dir2 = calloc(1, sizeof(*debug_info_dir2));
                        TAILQ_INIT(debug_info_dir2);
                    }

                    entry = calloc(1, sizeof(*entry));
                    entry->data = strdup(tmp);
                    TAILQ_INSERT_TAIL(debug_info_dir2, entry, items);
                }

                free(tmp);
            }
        }
    }

    return;
}

static void get_include_dirs(const struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    string_entry_t *entry = NULL;
    char *tmp = NULL;
    struct stat sb;

    assert(ri != NULL);

    TAILQ_FOREACH(peer, ri->peers, items) {
        if (headerIsSource(peer->before_hdr) || headerIsSource(peer->after_hdr)) {
            continue;
        }

        free(tmp);

        /* include dirs from the before build go in headers_dir1 */
        if (peer->before_files && !TAILQ_EMPTY(peer->before_files)) {
            xasprintf(&tmp, "%s%s", peer->before_root, ri->include_path);
            assert(tmp != NULL);
            memset(&sb, 0, sizeof(sb));

            if (lstat(tmp, &sb) == -1) {
                warn("lstat()");
                continue;
            }

            if (S_ISDIR(sb.st_mode)) {
                if (headers_dir1 == NULL) {
                    headers_dir1 = calloc(1, sizeof(*headers_dir1));
                    TAILQ_INIT(headers_dir1);
                }

                entry = calloc(1, sizeof(*entry));
                entry->data = strdup(tmp);
                TAILQ_INSERT_TAIL(headers_dir1, entry, items);
            }

            free(tmp);
        }

        free(tmp);

        /* include dirs from the after build go in headers_dir2 */
        if (peer->after_files && !TAILQ_EMPTY(peer->after_files)) {
            xasprintf(&tmp, "%s%s", peer->after_root, ri->include_path);
            assert(tmp != NULL);
            memset(&sb, 0, sizeof(sb));

            if (lstat(tmp, &sb) == -1) {
                warn("lstat()");
                continue;
            }

            if (S_ISDIR(sb.st_mode)) {
                if (headers_dir2 == NULL) {
                    headers_dir2 = calloc(1, sizeof(*headers_dir2));
                    TAILQ_INIT(headers_dir2);
                }

                entry = calloc(1, sizeof(*entry));
                entry->data = strdup(tmp);
                TAILQ_INSERT_TAIL(headers_dir2, entry, items);
            }

            free(tmp);
        }
    }

    return;
}

static bool abidiff_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    bool rebase = false;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* skip anything without a peer */
    if (file->peer_file == NULL) {
        return true;
    }

    /* skip anything that is not an ELF file */
    if (!S_ISREG(file->st.st_mode) && !is_elf(file->fullpath)) {
        return true;
    }

    /* build the abidiff command */

/*
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
    rebase = is_rebase(ri);

    init_result_params(&params);
    params.header = HEADER_ABIDIFF;





    return true;
}

/*
 * Main driver for the 'abidiff' inspection.
 */
bool inspect_abidiff(struct rpminspect *ri) {
    bool result = false;
    struct result_params params;

    assert(ri != NULL);

    /* if there's a .abignore file in the after SRPM, we need to use it */
    suppressions = get_suppressions(ri);

    /* get the debug info dirs */
    get_debuginfo_dirs(ri);

    /* get the header dirs */
    get_include_dirs(ri);

    /* run the main inspection */
    result = foreach_peer_file(ri, abidiff_driver, true);

    /* clean up */
    list_free(suppressions, free);
    list_free(debug_info_dir1, free);
    list_free(debug_info_dir2, free);
    list_free(headers_dir1, free);
    list_free(headers_dir2, free);

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
