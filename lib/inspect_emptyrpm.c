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
 * @file inspect_emptyrpm.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019-2020
 * @brief 'emptyrpm' inspection
 * @copyright LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "rpminspect.h"

/**
 * @brief Perform the 'emptyrpm' inspection.
 *
 * Report any packages that appear in the build that contain new
 * payload.  When comparing two builds, only report new empty payload
 * packages.
 *
 * @param ri Pointer to the struct rpminspect for the program.
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
     * The emptyrpm inspection looks for any packages missing
     * payloads.  These are packages (or new packages when comparing)
     * that lack any payloads.
     */

    /* Check the binary peers */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (is_payload_empty(peer->after_files) && peer->before_rpm == NULL) {
            xasprintf(&params.msg, _("New package %s is empty (no payloads)"), basename(peer->after_rpm));
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.remedy = REMEDY_EMPTYRPM;
            add_result(ri, &params);
            free(params.msg);

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
