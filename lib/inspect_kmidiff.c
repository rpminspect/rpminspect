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
#include <limits.h>
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

/*
 * Main driver for the 'kmidiff' inspection.
 */
bool inspect_kmidiff(struct rpminspect *ri) {
    bool result = false;
    string_entry_t *entry = NULL;
    size_t num_arches = 0;
    struct result_params params;

    assert(ri != NULL);

    /* if there's a .abignore file in the after SRPM, we need to use it */
    suppressions = get_abi_suppressions(ri, ri->kmidiff_suppression_file);

    /* get the debug info dirs (headers not used for kmidiff) */
    num_arches = list_len(ri->arches);
    debug_info_dir1_table = get_abi_dir_arg(ri, num_arches, DEBUGINFO_SUFFIX, ABI_DEBUG_INFO_DIR1, DEBUG_PATH, BEFORE_BUILD);
    debug_info_dir2_table = get_abi_dir_arg(ri, num_arches, DEBUGINFO_SUFFIX, ABI_DEBUG_INFO_DIR2, DEBUG_PATH, AFTER_BUILD);

    /* build the list of first command line arguments */
    firstargs = calloc(1, sizeof(*firstargs));
    assert(firstargs != NULL);
    TAILQ_INIT(firstargs);

    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    entry->data = strdup(KMIDIFF_CMD);
    TAILQ_INSERT_TAIL(firstargs, entry, items);

    if (ri->kmidiff_extra_args) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->data = strdup(ri->kmidiff_extra_args);
        TAILQ_INSERT_TAIL(firstargs, entry, items);
    }

    /* clean up */
    list_free(firstargs, free);
    list_free(suppressions, free);
    free_argv_table(ri, debug_info_dir1_table);
    free_argv_table(ri, debug_info_dir2_table);

    /* report the inspection results */
    if (result) {
        init_result_params(&params);
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_ABIDIFF;
        params.severity = RESULT_OK;
        add_result(ri, &params);
    }

    return result;
}
