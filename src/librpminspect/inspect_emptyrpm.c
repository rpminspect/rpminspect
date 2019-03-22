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
    assert(filelist != NULL);

    /* Make sure the file list has at least one entry */
    return TAILQ_EMPTY(filelist);
}

/*
 * Main driver for the 'emptyrpm' inspection.
 */
bool inspect_emptyrpm(struct rpminspect *ri) {
    bool ret = false;
    rpmpeer_entry_t *peer = NULL;
    char *msg = NULL;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    /*
     * The emptyrpm inspection looks for any packages missing payloads.
     * These could be packages that lost their payloads from the before
     * build to the after build or new packages that lack any payloads.
     */

    /* Check the binary peers */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (is_payload_empty(peer->after_files)) {
            if (peer->before_rpm == NULL) {
                xasprintf(&msg, "New package %s is empty (no payloads)", basename(peer->after_rpm));

                add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_EMPTYRPM, msg, NULL, REMEDY_EMPTYRPM);

                free(msg);
            } else if (is_payload_empty(peer->before_files)) {
                if (ri->verbose) {
                    xasprintf(&msg, "Package %s continues to be empty (no payloads)", basename(peer->after_rpm));

                    add_result(&ri->results, RESULT_INFO, NOT_WAIVABLE, HEADER_EMPTYRPM, msg, NULL, REMEDY_EMPTYRPM);

                    free(msg);
                }
            } else {
                xasprintf(&msg, "Package %s became empty (no payloads)", basename(peer->after_rpm));

                add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_EMPTYRPM, msg, NULL, REMEDY_EMPTYRPM);

                free(msg);
            }

            ret = true;
        }
    }

    return ret;
}
