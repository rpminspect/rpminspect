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

#include <stdio.h>
#include <assert.h>
#include <rpm/header.h>

#include "rpminspect.h"

static bool pathmigration_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    string_map_t *hentry = NULL;
    string_map_t *tmp_hentry = NULL;
    string_entry_t *entry = NULL;
    char *old = NULL;
    const char *arch = NULL;
    struct result_params params;

    /* Ignore files in the SRPM */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Skip files beginning with an excluded path */
    if (ri->pathmigration_excluded_paths && !TAILQ_EMPTY(ri->pathmigration_excluded_paths)) {
        TAILQ_FOREACH(entry, ri->pathmigration_excluded_paths, items) {
            /* check in case of an exact match */
            if (!strcmp(file->localpath, entry->data)) {
                return true;
            }

            /* ensure the path prefixes end with '/' */
            if (strsuffix(entry->data, "/")) {
                old = strdup(entry->data);
            } else {
                xasprintf(&old, "%s/", entry->data);
            }

            /* if matched, return */
            if (strprefix(file->localpath, entry->data)) {
                free(old);
                return true;
            }

            free(old);
        }
    }

    /* Used for reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_VERIFY;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = HEADER_PATHMIGRATION;
    params.remedy = REMEDY_PATHMIGRATION;
    params.arch = arch;

    /* Check for each path migration, break early if we find a match */
    HASH_ITER(hh, ri->pathmigration, hentry, tmp_hentry) {
        /*
         * Make sure the old path name ends with a slash.
         */
        if (strsuffix(hentry->key, "/")) {
            old = strdup(hentry->key);
        } else {
            xasprintf(&old, "%s/", hentry->key);
        }

        DEBUG_PRINT("hentry->key=|%s|, hentry->value=|%s|, old=|%s|, file->localpath=|%s|\n", hentry->key, hentry->value, old, file->localpath);

        /* Check to see if we found a path that should be migrated */
        if (strprefix(file->localpath, old)) {
            xasprintf(&params.msg, "File %s found should be in %s on %s", file->localpath, hentry->value, arch);
            params.file = file->localpath;
            add_result(ri, &params);
            free(params.msg);
            result = false;
        }

        free(old);

        if (!result) {
            break;
        }
    }

    return result;
}

/*
 * Main driver for the 'pathmigration' inspection.
 */
bool inspect_pathmigration(struct rpminspect *ri) {
    bool result = true;
    struct result_params params;

    assert(ri != NULL);

    /* Only run the inspection if path migrations are specified */
    if (ri->pathmigration) {
        result = foreach_peer_file(ri, pathmigration_driver, true);
    }

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_PATHMIGRATION;
        add_result(ri, &params);
    }

    return result;
}
