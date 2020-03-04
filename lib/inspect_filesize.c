/*
 * Copyright (C) 2019  Red Hat, Inc.
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
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool filesize_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    off_t change;
    char *msg = NULL;
    severity_t sev = RESULT_INFO;
    waiverauth_t waiver = NOT_WAIVABLE;

    assert(ri != NULL);
    assert(file != NULL);

    /* Files without a peer have to be ignored */
    if (file->peer_file == NULL) {
        return true;
    }

    /* Ignore files in the SRPM */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Only run this check on regular files */
    if (!S_ISREG(file->st.st_mode) && !S_ISREG(file->peer_file->st.st_mode)) {
        return true;
    }

    /* Nothing to do if the sizes are the same */
    if (file->st.st_size == file->peer_file->st.st_size) {
        return true;
    }

    /* We need the architecture for reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Size checks and messaging */
    if (file->st.st_size > 0 && file->peer_file->st.st_size == 0) {
        /* became non-empty */
        xasprintf(&msg, _("%s became a non-empty file on %s"), file->localpath, arch);
        sev = RESULT_VERIFY;
        waiver = WAIVABLE_BY_ANYONE;
        result = false;
    } else if (file->st.st_size == 0 && file->peer_file->st.st_size > 0) {
        /* became empty */
        xasprintf(&msg, _("%s became an empty file on %s"), file->localpath, arch);
        sev = RESULT_VERIFY;
        waiver = WAIVABLE_BY_ANYONE;
        result = false;
    } else {
        change = ((file->st.st_size - file->peer_file->st.st_size) / file->peer_file->st.st_size) * 100;

        if (change > 0) {
            /* file grew */
            xasprintf(&msg, _("%s grew by +%ld%% on %s"), file->localpath, change, arch);
        } else if (change < 0) {
            /* file shrank */
            xasprintf(&msg, _("%s shrank by -%ld%% on %s"), file->localpath, change, arch);
        }
    }

    /* Reporting */
    if (msg) {
        add_result(ri, sev, waiver, HEADER_FILESIZE, msg, NULL, NULL);
        free(msg);
    }

    return result;
}

/*
 * Main driver for the 'filesize' inspection.
 */
bool inspect_filesize(struct rpminspect *ri) {
    bool result;

    assert(ri != NULL);

    /* run the size inspection across all RPM files */
    result = foreach_peer_file(ri, filesize_driver);

    /* if everything was fine, just say so */
    if (result) {
        add_result(ri, RESULT_OK, NOT_WAIVABLE, HEADER_FILESIZE, NULL, NULL, NULL);
    }

    return result;
}
