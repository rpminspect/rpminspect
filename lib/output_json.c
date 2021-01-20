/*
 * Copyright (C) 2019-2021  Red Hat, Inc.
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
#include <json.h>

#include "rpminspect.h"

/*
 * Output a results_t in JSON format.
 */
void output_json(const results_t *results, const char *dest, __attribute__((unused)) const severity_t threshold) {
    results_entry_t *result = NULL;
    int r = 0;
    int len = 0;
    FILE *fp = NULL;
    const char *header = NULL;
    struct json_object *j = NULL;
    struct json_object *ji = NULL;
    struct json_object *jr = NULL;
    int flags = JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY;
    struct json_object_iter iter;

    assert(results != NULL);

    /*
     * The main results object.  Each inspection will be an array
     * with the results contained as array elements.
     */
    j = json_object_new_object();

    /* output the results */
    TAILQ_FOREACH(result, results, items) {
        /* if we have a new inspection set, create a new object */
        if (header == NULL || strcmp(header, result->header)) {
            /* add previous inspection if there is one */
            if (ji != NULL) {
                json_object_object_add(j, header, json_object_get(ji));
            }

            /* new inspection begins now */
            header = result->header;
            ji = json_object_new_array();
        }

        /* create the object for this result */
        jr = json_object_new_object();
        json_object_object_add(jr, "result", json_object_new_string(strseverity(result->severity)));
        json_object_object_add(jr, "waiver authorization", json_object_new_string(strwaiverauth(result->waiverauth)));

        if (result->msg != NULL) {
            json_object_object_add(jr, "message", json_object_new_string(result->msg));
        }

        if (result->details != NULL) {
            json_object_object_add(jr, "details", json_object_new_string(result->details));
        }

        if (result->remedy != NULL) {
            json_object_object_add(jr, "remedy", json_object_new_string(result->remedy));
        }

        /* add this result data to the inspection array */
        json_object_array_add(ji, jr);
    }

    /* add the final inspection */
    json_object_object_add(j, header, json_object_get(ji));

    /* default to stdout unless a filename was specified */
    if (dest == NULL) {
        fp = stdout;
    } else {
        fp = fopen(dest, "w");

        if (fp == NULL) {
            fprintf(stderr, _("*** Error opening %s for writing: %s\n"), dest, strerror(errno));
            fflush(stderr);
            return;
        }
    }

    /* write out the results */
    fprintf(fp, "%s\n", json_object_to_json_string_ext(j, flags));

    /* tidy up and return */
    r = fflush(fp);
    assert(r == 0);

    if (dest != NULL) {
        r = fclose(fp);
        assert(r == 0);
    }

    /* clean up the json library memory allocation */
    json_object_object_foreachC(j, iter) {
        if (json_object_get_type(iter.val) == json_type_array) {
            len = json_object_array_length(iter.val);

            for (r = 0; r < len; r++) {
                json_object_array_put_idx(iter.val, r, NULL);
            }
        }

        json_object_put(iter.val);
    }

    json_object_put(j);

    return;
}
