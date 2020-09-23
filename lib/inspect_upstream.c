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
#include <assert.h>
#include <string.h>
#include "queue.h"
#include "rpminspect.h"

/* Global variables */
static string_list_t *source = NULL;
static bool initialized = false;
static struct result_params params;

/* Returns true if this file is a Source file */
static bool is_source(const rpmfile_entry_t *file)
{
    bool ret = false;
    char *shortname = NULL;
    string_entry_t *entry = NULL;

    assert(file != NULL);

    /* Initialize the source list once for the run */
    if (!initialized) {
        source = get_rpm_header_string_array(file->rpm_header, RPMTAG_SOURCE);
        initialized = true;
    }

    if (source == NULL) {
        /* source package lacks any Source archives */
        return false;
    }

    /* The RPM header stores basenames */
    shortname = rindex(file->fullpath, '/') + 1;

    /* See if this file is a Source file */
    TAILQ_FOREACH(entry, source, items) {
        if (!strcmp(shortname, entry->data)) {
            return true;
        }
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
        result = false;
    } else {
        /* compare checksums to see if the upstream sources changed */
        before_sum = checksum(file->peer_file);
        after_sum = checksum(file);

        if (strcmp(before_sum, after_sum)) {
            /* capture 'diff -u' output for text files */
            if (is_text_file(file->peer_file) && is_text_file(file)) {
                diff_head = diff_output = run_cmd(&exitcode, DIFF_CMD, "-u", file->peer_file->fullpath, file->fullpath, NULL);

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
            result = false;

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
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;

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

    /* Set result type based on version difference */
    if (is_rebase(ri)) {
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
        if (!headerIsSource(peer->after_hdr)) {
            continue;
        }

        /* On the off chance the SRPM is empty, just ignore */
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        /* Iterate over the SRPM files */
        TAILQ_FOREACH(file, peer->after_files, items) {
            if (!upstream_driver(ri, file)) {
                result = false;
            }
        }

        /* Report any removed source files from the SRPM */
        if (peer->before_files) {
            TAILQ_FOREACH(file, peer->before_files, items) {
                if (file->peer_file == NULL) {
                    xasprintf(&params.msg, _("Source file `%s` removed"), file->localpath);
                    add_result(ri, &params);
                    free(params.msg);
                    result = false;
                }
            }
        }
    }

    /* Sound the everything-is-ok alarm if everything is, in fact, ok */
    if (result) {
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.remedy = NULL;
        add_result(ri, &params);
    }

    /* Our static list of SourceN: spec file members, dump it */
    list_free(source, free);

    return result;
}
