/*
 * Copyright (C) 2020  Red Hat, Inc.
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

#include <stdio.h>
#include <assert.h>
#include <rpm/header.h>

#include "rpminspect.h"

static bool pathmigration_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    ENTRY e;
    ENTRY *eptr = NULL;
    string_entry_t *entry = NULL;
    char *old = NULL;
    const char *arch = NULL;
    struct result_params params;

    /* Ignore files in the SRPM */
    if (headerIsSource(file->rpm_header)) {
        return true;
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
    TAILQ_FOREACH(entry, ri->pathmigration_keys, items) {
        e.key = entry->data;
        hsearch_r(e, FIND, &eptr, ri->pathmigration);

        if (eptr == NULL) {
            continue;
        }

        /*
         * Make sure the old path name ends with a slash.
         */

        if (strsuffix(entry->data, "/")) {
            old = strdup(entry->data);
        } else {
            xasprintf(&old, "%s/", entry->data);
        }

        DEBUG_PRINT("old=|%s|, file->localpath=|%s|\n", old, file->localpath);

        /* Check to see if we found a path that should be migrated */
        if (strprefix(file->localpath, old)) {
            xasprintf(&params.msg, "File %s found should be in %s on %s", file->localpath, (char *) eptr->data, arch);
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
    if (ri->pathmigration_keys && !TAILQ_EMPTY(ri->pathmigration_keys)) {
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
