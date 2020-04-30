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

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <archive.h>
#include <archive_entry.h>
#include "rpminspect.h"

/*
 * Given the filelist for an rpm, is the payload empty?
 * All we do here is count the payload entries.  If we have more than zero, the
 * payload is not empty.
 */
bool is_payload_empty(rpmfile_t *filelist) {
    if (filelist == NULL) {
        return true;
    }

    /* Make sure the file list has at least one entry */
    return TAILQ_EMPTY(filelist);
}

/*
 * Main driver for the 'emptyrpm' inspection.
 */
bool inspect_emptyrpm(struct rpminspect *ri) {
    bool good = true;
    rpmpeer_entry_t *peer = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    init_result_params(&params);
    params.header = HEADER_EMPTYRPM;

    /*
     * The emptyrpm inspection looks for any packages missing payloads.
     * These could be packages that lost their payloads from the before
     * build to the after build or new packages that lack any payloads.
     */

    /* Check the binary peers */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /*
         * Subpackages may disappear in subsequent builds.  Sometimes this
         * is intentional, sometimes not.
         */
        if (peer->before_rpm != NULL && peer->after_rpm == NULL) {
            xasprintf(&params.msg, _("Existing subpackage %s is now missing"), headerGetString(peer->before_hdr, RPMTAG_NAME));
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.remedy = REMEDY_EMPTYRPM;
            add_result(ri, &params);
            free(params.msg);
            good = false;
            continue;
        }

        if (is_payload_empty(peer->after_files)) {
            if (peer->before_rpm == NULL) {
                xasprintf(&params.msg, _("New package %s is empty (no payloads)"), basename(peer->after_rpm));
                params.severity = RESULT_VERIFY;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.remedy = REMEDY_EMPTYRPM;
                add_result(ri, &params);
                free(params.msg);
            } else if (is_payload_empty(peer->before_files)) {
                xasprintf(&params.msg, _("Package %s continues to be empty (no payloads)"), basename(peer->after_rpm));
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
                params.remedy = REMEDY_EMPTYRPM;
                add_result(ri, &params);
                free(params.msg);
            } else {
                xasprintf(&params.msg, _("Package %s became empty (no payloads)"), basename(peer->after_rpm));
                params.severity = RESULT_VERIFY;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.remedy = REMEDY_EMPTYRPM;
                add_result(ri, &params);
                free(params.msg);
            }

            good = false;
        }
    }

    if (good) {
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        add_result(ri, &params);
    }

    return good;
}
