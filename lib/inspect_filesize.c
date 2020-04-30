/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
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
#include <limits.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static long int threshold = 0;

static bool filesize_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    off_t change;
    struct result_params params;

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

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.header = HEADER_FILESIZE;
    params.arch = arch;
    params.file = file->localpath;

    /* Size checks and messaging */
    if (file->st.st_size > 0 && file->peer_file->st.st_size == 0) {
        /* became non-empty */
        xasprintf(&params.msg, _("%s became a non-empty file on %s"), file->localpath, arch);
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_CHANGED;
        params.noun = _("non-empty ${FILE}");
        result = false;
    } else if (file->st.st_size == 0 && file->peer_file->st.st_size > 0) {
        /* became empty */
        xasprintf(&params.msg, _("%s became an empty file on %s"), file->localpath, arch);
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_CHANGED;
        params.noun = _("empty ${FILE}");
        result = false;
    } else {
        change = ((file->st.st_size - file->peer_file->st.st_size) / file->peer_file->st.st_size) * 100;
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;

        if (threshold > 0 && (labs(change) >= threshold)) {
            /* change is at or above our reporting change threshold for VERIFY */
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.verb = VERB_CHANGED;
            result = false;
        }

        if (change > 0) {
            /* file grew */
            xasprintf(&params.msg, _("%s grew by +%ld%% on %s"), file->localpath, change, arch);
            params.noun = _("${FILE} size grew");
        } else if (change < 0) {
            /* file shrank */
            xasprintf(&params.msg, _("%s shrank by -%ld%% on %s"), file->localpath, change, arch);
            params.noun = _("${FILE} size shrank");
        }
    }

    /* Reporting */
    if (params.msg) {
        add_result(ri, &params);
        free(params.msg);
    }

    return result;
}

/*
 * Main driver for the 'filesize' inspection.
 */
bool inspect_filesize(struct rpminspect *ri) {
    bool result;
    struct result_params params;

    assert(ri != NULL);

    /* if there is a size reporting threshold, get it */
    if (ri->size_threshold) {
        threshold = strtol(ri->size_threshold, 0, 10);

        if ((threshold == LONG_MIN || threshold == LONG_MAX) && errno == ERANGE) {
            /* unable to use the threshold, ignore */
            DEBUG_PRINT("unable to use size_threshold=|%s|\n", ri->size_threshold);
            threshold = 0;
        }
    }

    /* run the size inspection across all RPM files */
    result = foreach_peer_file(ri, filesize_driver);

    /* if everything was fine, just say so */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_FILESIZE;
        add_result(ri, &params);
    }

    return result;
}
