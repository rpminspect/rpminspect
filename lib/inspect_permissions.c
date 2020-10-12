/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool permissions_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    const char *what = _("changed");
    mode_t before_mode;
    mode_t after_mode;
    mode_t mode_diff;
    bool allowed = false;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Ignore files in the SRPM */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* We need the architecture for reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_VERIFY;
    params.header = HEADER_PERMISSIONS;
    params.arch = arch;

    /* Local working copies for display */
    after_mode = file->st.st_mode & 0777;

    /* Compare the modes */
    if (file->peer_file) {
        before_mode = file->peer_file->st.st_mode & 0777;
    } else {
        memset(&before_mode, 0, sizeof(before_mode));
    }

    mode_diff = before_mode ^ after_mode;
    allowed = match_fileinfo_mode(ri, file, HEADER_PERMISSIONS, NULL);

    /* if setuid/setgid or new mode is more open */
    if (mode_diff && file->peer_file && !allowed) {
        if (!(before_mode & (S_ISUID|S_ISGID)) && (after_mode & (S_ISUID|S_ISGID))) {
            params.severity = RESULT_BAD;
            what = _("changed setuid/setgid");
        } else if (S_ISDIR(file->st.st_mode) && !S_ISLNK(file->st.st_mode)) {
            if (((mode_diff & S_ISVTX) && !(after_mode & S_ISVTX)) || ((after_mode & mode_diff) != 0)) {
                params.severity = RESULT_BAD;
                what = _("relaxed");
            }
        }

        xasprintf(&params.msg, _("%s %s permissions from %04o to %04o on %s"), file->localpath, what, before_mode, after_mode, arch);
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.file = file->localpath;
        add_result(ri, &params);
        free(params.msg);

        result = false;
    }

    /* check for world-writability */
    if (!allowed && !S_ISLNK(file->st.st_mode) && ((after_mode & (S_IWOTH|S_ISVTX)) || (after_mode & S_IWOTH))) {
        xasprintf(&params.msg, _("%s (%s) is world-writable on %s"), file->localpath, get_mime_type(file), arch);
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_SECURITY;
        add_result(ri, &params);
        free(params.msg);
        result = false;
    }

    return result;
}

/*
 * Main driver for the 'permissions' inspection.
 */
bool inspect_permissions(struct rpminspect *ri) {
    bool result = false;
    struct result_params params;

    assert(ri != NULL);

    /* run the permissions inspection across all RPM files */
    result = foreach_peer_file(ri, permissions_driver, true);

    /* if everything was fine, just say so */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_PERMISSIONS;
        add_result(ri, &params);
    }

    return result;
}
