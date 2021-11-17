/*
 * Copyright © 2019 Red Hat, Inc.
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

/**
 * @file inspect_lostpayload.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019-2020
 * @brief 'lostpayload' inspection
 * @copyright LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "rpminspect.h"

/**
 * @brief Perform the 'lostpayload' inspection.
 *
 * Check all binary RPMs in the before and after builds for any
 * packages that lose their payloads from the before build to the
 * after build.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_lostpayload(struct rpminspect *ri) {
    bool good = true;
    bool messaged = false;
    rpmpeer_entry_t *peer = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    init_result_params(&params);
    params.header = NAME_LOSTPAYLOAD;

    /*
     * The lostpayload inspection looks for any packages missing payloads.
     * These could be packages that lost their payloads from the before
     * build to the after build.
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
            params.remedy = REMEDY_LOSTPAYLOAD;
            add_result(ri, &params);
            free(params.msg);
            good = false;
            messaged = true;
            continue;
        }

        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            if (peer->before_files == NULL || TAILQ_EMPTY(peer->before_files)) {
                xasprintf(&params.msg, _("Package %s continues to be empty (no payloads)"), basename(peer->after_rpm));
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
                params.remedy = REMEDY_LOSTPAYLOAD;
                add_result(ri, &params);
                free(params.msg);
            } else {
                xasprintf(&params.msg, _("Package %s became empty (no payloads)"), basename(peer->after_rpm));
                params.severity = RESULT_VERIFY;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.remedy = REMEDY_LOSTPAYLOAD;
                add_result(ri, &params);
                free(params.msg);
                good = false;
            }

            messaged = true;
        }
    }

    if (!messaged) {
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        add_result(ri, &params);
    }

    return good;
}
