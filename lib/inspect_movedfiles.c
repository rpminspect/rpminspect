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

#include <assert.h>
#include "rpminspect.h"

static bool movedfiles_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    bool rebase = false;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* skip anything without a peer */
    if (file->peer_file == NULL) {
        return true;
    }

    /* skip anything that has not moved */
    if (!file->moved_path && !file->peer_file->moved_path) {
        return true;
    }

    /* determine if this is a rebase build */
    rebase = is_rebase(ri);

    init_result_params(&params);
    params.header = HEADER_MOVEDFILES;

    if (rebase) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
    } else {
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
    }

    xasprintf(&params.msg, "%s probably moved to %s on %s\n", file->peer_file->localpath, file->localpath, get_rpm_header_arch(file->rpm_header));
    add_result(ri, &params);
    free(params.msg);
    return false;
}

/*
 * Main driver for the 'movedfiles' inspection.
 */
bool inspect_movedfiles(struct rpminspect *ri) {
    bool result = false;
    struct result_params params;

    assert(ri != NULL);

    result = foreach_peer_file(ri, movedfiles_driver, true);

    init_result_params(&params);
    params.waiverauth = NOT_WAIVABLE;
    params.header = HEADER_MOVEDFILES;

    if (result) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    }

    return result;
}
