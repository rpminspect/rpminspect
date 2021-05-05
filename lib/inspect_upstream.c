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

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "queue.h"
#include "rpminspect.h"

/* Global variables */
static string_list_t *source = NULL;
static bool reported = false;
static struct result_params params;

/* Returns true if this file is a Source file */
static bool is_source(const rpmfile_entry_t *file)
{
    bool ret = false;
    char *shortname = NULL;

    assert(file != NULL);

    if (source == NULL || TAILQ_EMPTY(source)) {
        /* source package lacks any Source archives */
        return false;
    }

    /* The RPM header stores basenames */
    shortname = rindex(file->fullpath, '/') + 1;

    /* See if this file is a Source file */
    if (list_contains(source, shortname)) {
        return true;
    }

    return ret;
}

/* Main driver for the 'upstream' inspection. */
static bool upstream_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    char *before_sum = NULL;
    char *after_sum = NULL;
    char *diff_output = NULL;
    char *diff_head = NULL;
    int exitcode;

    /* If we are not looking at a Source file, bail. */
    if (!is_source(file)) {
        return true;
    }

    /* Compare digests of source archive */
    params.file = file->localpath;

    if (file->peer_file == NULL) {
        xasprintf(&params.msg, _("New upstream source file `%s` appeared"), params.file)
        params.verb = VERB_ADDED;
        params.noun = _("source file ${FILE}");
        add_result(ri, &params);
        result = !(params.severity >= RESULT_VERIFY);
        reported = true;
    } else {
        /* compare checksums to see if the upstream sources changed */
        before_sum = checksum(file->peer_file);
        after_sum = checksum(file);

        if (strcmp(before_sum, after_sum)) {
            /* capture 'diff -u' output for text files */
            if (is_text_file(file->peer_file) && is_text_file(file)) {
                diff_head = diff_output = run_cmd(&exitcode, ri->commands.diff, "-u", file->peer_file->fullpath, file->fullpath, NULL);

                /* skip the two leading lines */
                if (strprefix(diff_head, "--- ")) {
                    diff_head = index(diff_head, '\n') + 1;
                }

                if (strprefix(diff_head, "+++ ")) {
                    diff_head = index(diff_head, '\n') + 1;
                }
            }

            /* report the changed file */
            xasprintf(&params.msg, _("Upstream source file `%s` changed content"), params.file);
            params.verb = VERB_CHANGED;
            params.noun = _("checksum of ${FILE}");
            add_result(ri, &params);
            result = !(params.severity >= RESULT_VERIFY);
            reported = true;

            /* clean up */
            free(diff_output);
        }
    }

    free(params.msg);
    params.msg = NULL;

    return result;
}

/*
 * Main driver for the 'upstream' inspection.
 */
bool inspect_upstream(struct rpminspect *ri)
{
    bool result = true;
    bool have_source = false;
    const char *name = NULL;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    string_list_t *before_source = NULL;
    string_list_t *removed = NULL;
    string_entry_t *entry = NULL;

    assert(ri != NULL);

    init_result_params(&params);
    params.header = HEADER_UPSTREAM;

    /* Check for source package */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (headerIsSource(peer->before_hdr) && headerIsSource(peer->after_hdr)) {
            have_source = true;
            break;
        }
    }

    /* If no versions found, we are not looking at source packages */
    if (!have_source) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        xasprintf(&params.msg, _("No source packages available, skipping inspection."));
        add_result(ri, &params);
        free(params.msg);
        return result;
    }

    name = headerGetString(peer->after_hdr, RPMTAG_NAME);
    assert(name != NULL);

    /* Set result type based on version difference */
    if (is_rebase(ri) || (init_rebaseable(ri) && list_contains(ri->rebaseable, name))) {
        /* versions changed */
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.remedy = NULL;
    } else {
        /* versions are the same, likely maintenance */
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.remedy = REMEDY_UPSTREAM;
    }

    /* Run the main inspection */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Only look at the files in SRPMs */
        if (peer->after_rpm == NULL || !headerIsSource(peer->after_hdr)) {
            continue;
        }

        /* Get the list of source files from each build */
        before_source = get_rpm_header_string_array(peer->before_hdr, RPMTAG_SOURCE);
        source = get_rpm_header_string_array(peer->after_hdr, RPMTAG_SOURCE);

        /* Iterate over the SRPM files */
        TAILQ_FOREACH(file, peer->after_files, items) {
            if (!upstream_driver(ri, file)) {
                result = false;
            }
        }

        /* Report any removed source files from the SRPM */
        removed = list_difference(before_source, source);

        if (removed != NULL && !TAILQ_EMPTY(removed)) {
            TAILQ_FOREACH(entry, removed, items) {
                xasprintf(&params.msg, _("Source file `%s` removed"), entry->data);
                add_result(ri, &params);
                free(params.msg);
                result = !(params.severity >= RESULT_VERIFY);
                reported = true;
            }
        }

        list_free(removed, free);
        list_free(before_source, free);
        list_free(source, free);
    }

    /* Sound the everything-is-ok alarm if everything is, in fact, ok */
    if (result && !reported) {
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.msg = NULL;
        params.remedy = NULL;
        add_result(ri, &params);
    }

    return result;
}
