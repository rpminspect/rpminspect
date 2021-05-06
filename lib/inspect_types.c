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

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "rpminspect.h"

static bool result = true;

static bool types_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    const char *arch = NULL;
    char *bt = NULL;
    char *at = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Skip debuginfo and debugsource packages */
    if (is_debug_or_build_path(file->localpath)) {
        return true;
    }

    /* Files without a peer have to be ignored */
    if (file->peer_file == NULL) {
        return true;
    }

    /* Only run this check on regular files */
    if (!S_ISREG(file->st.st_mode) && !S_ISREG(file->peer_file->st.st_mode)) {
        return true;
    }

    /* We need the architecture for reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Get the MIME types */
    bt = get_mime_type(file->peer_file);
    at = get_mime_type(file);

    DEBUG_PRINT("before_type=|%s|, after_type=|%s| -- %s\n", bt, at, file->localpath);

    /* Compare */
    if (strcmp(bt, at)) {
        init_result_params(&params);
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.header = NAME_TYPES;
        params.remedy = REMEDY_TYPES;
        params.arch = arch;
        params.file = file->localpath;
        params.verb = VERB_CHANGED;
        params.noun = _("${FILE} MIME type");
        xasprintf(&params.msg, _("MIME type on %s was %s and became %s on %s"), file->localpath, bt, at, arch);
        add_result(ri, &params);
        free(params.msg);

        result = false;
    }

    return result;
}

/*
 * Main driver for the 'types' inspection.
 */
bool inspect_types(struct rpminspect *ri) {
    struct result_params params;

    assert(ri != NULL);

    /* run the types inspection across all RPM files */
    foreach_peer_file(ri, NAME_TYPES, types_driver, true);

    /* if everything was fine, just say so */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_TYPES;
        add_result(ri, &params);
    }

    return result;
}
