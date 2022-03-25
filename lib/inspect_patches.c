/*
 * Copyright 2020 Red Hat, Inc.
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <limits.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>
#include <archive.h>
#include "queue.h"
#include "rpminspect.h"

/* Global variables */
static string_list_t *patches = NULL;
static bool initialized = false;
static bool reported = false;
static bool comparison = false;
static struct result_params params;

enum { DIFF_NULL, DIFF_CONTEXT, DIFF_UNIFIED };

/* Returns true if this file is a Patch file */
static bool is_patch(const rpmfile_entry_t *file)
{
    char *shortname = NULL;

    assert(file != NULL);

    /* Initialize the patches list once for the run */
    if (!initialized) {
        patches = get_rpm_header_string_array(file->rpm_header, RPMTAG_PATCH);
        initialized = true;
    }

    if (patches == NULL) {
        /* source RPM does not contain any Patch entries */
        return false;
    }

    /* The RPM header stores basenames */
    shortname = rindex(file->fullpath, '/') + 1;

    /* See if this file is a Patch file */
    return list_contains(patches, shortname);
}

/*
 * Compute number of files and lines changed in a patch.
 */
static patchstat_t get_patch_stats(const char *patch)
{
    patchstat_t r;
    string_list_t *lines = NULL;
    string_entry_t *line = NULL;
    int difftype = DIFF_NULL;
    bool maybe_context = false;
    bool maybe_unified = false;
    int header_count = -1;

    assert(patch != NULL);

    /* initialize our counts */
    r.files = 0;
    r.lines = 0;

    /* read in the patch file */
    lines = read_file(patch);

    if (lines == NULL || TAILQ_EMPTY(lines)) {
        return r;
    }

    /* iterate over each line counting files and line modifications */
    TAILQ_FOREACH(line, lines, items) {
        if (difftype == DIFF_NULL) {
            if (!maybe_context && !maybe_unified) {
                if (strprefix(line->data, "*** ")) {
                    header_count++;
                    maybe_context = true;
                } else if (strprefix(line->data, "--- ")) {
                    header_count++;
                    maybe_unified = true;
                }
            } else if (maybe_context && !maybe_unified) {
                if (strprefix(line->data, "--- ")) {
                    header_count++;
                } else if (header_count == 1 && strprefix(line->data, "**********")) {
                    r.files++;
                    difftype = DIFF_CONTEXT;
                    header_count = -1;
                    maybe_context = false;
                }
            } else if (!maybe_context && maybe_unified) {
                if (strprefix(line->data, "+++ ")) {
                    header_count++;
                } else if (header_count == 1 && strprefix(line->data, "@@ ")) {
                    r.files++;
                    difftype = DIFF_UNIFIED;
                    header_count = -1;
                    maybe_unified = false;
                }
            }
        } else if (difftype == DIFF_CONTEXT) {
            if (strprefix(line->data, "+ ") || strprefix(line->data, "- ")) {
                r.lines++;
            } else if (strprefix(line->data, "*** ")) {
                difftype = DIFF_NULL;
                header_count++;
                maybe_context = true;
            }
        } else if (difftype == DIFF_UNIFIED) {
            if ((strprefix(line->data, "+") || strprefix(line->data, "-")) && !strprefix(line->data, "--- ")) {
                r.lines++;
            } else if (strprefix(line->data, "--- ")) {
                difftype = DIFF_NULL;
                header_count++;
                maybe_unified = true;
            }
        }
    }

    /* clean up */
    list_free(lines, free);

    return r;
}

/* Main driver for the 'patches' inspection. */
static bool patches_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    char *before_patch = NULL;
    char *after_patch = NULL;
    char *details = NULL;
    patchstat_t ps;
    struct stat sb;
    size_t apsz = 0;
    size_t bpsz = 0;
    long unsigned int oldsize = 0;
    long unsigned int newsize = 0;

    /* If we are not looking at a Patch file, bail. */
    if (!is_patch(file)) {
        return true;
    }

    /* If this patch is on the ignore list, skip */
    if (list_contains(ri->patch_ignore_list, file->localpath)) {
        DEBUG_PRINT("Per the configuration file, ignoring %s\n", file->localpath);
        return true;
    }

    /* patches may be compressed, so uncompress them here for diff(1) */
    if (file->peer_file) {
        before_patch = uncompress_file(ri, file->peer_file->fullpath, NAME_PATCHES);

        if (before_patch == NULL) {
            warnx(_("unable to uncompress patch: %s"), file->peer_file->localpath);
            return false;
        }
    }

    after_patch = uncompress_file(ri, file->fullpath, NAME_PATCHES);

    if (after_patch == NULL) {
        warnx(_("unable to uncompress patch: %s"), file->localpath);
        return false;
    }

    /*
     * Ensure that all patches are at least 4 bytes in size, trapping
     * "empty patch" mistakes that have occurred when people are
     * generating multiple patches against multiple branches.
     */
    if (stat(after_patch, &sb) != 0) {
        warn("stat");
        return false;
    }

    apsz = sb.st_size;

    if (file->peer_file && before_patch && stat(before_patch, &sb) != 0) {
        warn("stat");
        return false;
    }

    bpsz = sb.st_size;

    if ((apsz < 4) || (bpsz < 4)) {
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.details = NULL;
        params.remedy = REMEDY_PATCHES_CORRUPT;
        params.verb = VERB_FAILED;
        params.noun = _("corrupt patch ${FILE}");

        if (apsz < 4) {
            params.file = file->localpath;
            xasprintf(&params.msg, _("Patch %s is under 4 bytes in size - is it corrupt?"), file->localpath);
            add_result(ri, &params);
            free(params.msg);
        }

        if (file->peer_file && before_patch && bpsz < 4) {
            params.file = file->peer_file->localpath;
            xasprintf(&params.msg, _("Patch %s is under 4 bytes in size - is it corrupt?"), file->peer_file->localpath);
            add_result(ri, &params);
            free(params.msg);
        }

        reported = true;
        free(after_patch);
        free(before_patch);
        return false;
    }

    /*
     * compare the patches if we have two builds
     * This just reports patches that change content.  It uses the INFO reporting level.
     */
    if (comparison && file->peer_file) {
        params.details = get_file_delta(before_patch, after_patch);

        if (params.details) {
            /* more than whitespace changed */
            oldsize = file->peer_file->st.st_size;
            newsize = file->st.st_size;
            xasprintf(&params.msg, _("%s changed (%ld bytes -> %ld bytes)"), file->localpath, oldsize, newsize);
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
            params.verb = VERB_CHANGED;
            params.noun = _("patch file ${FILE}");
            params.file = file->localpath;

            /* use friendly names for the files in the diff(1) details */
            details = strreplace(params.details, before_patch, file->peer_file->localpath);
            assert(details != NULL);
            free(params.details);
            params.details = strreplace(details, after_patch, file->localpath);
            free(details);
            assert(params.details != NULL);

            /* report the findings */
            add_result(ri, &params);
            free(params.details);
            free(params.msg);
            params.details = NULL;
            params.msg = NULL;

            reported = true;
        }
    } else if (comparison && file->peer_file == NULL) {
        xasprintf(&params.msg, _("New patch file `%s` appeared"), params.file);
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.verb = VERB_ADDED;
        params.noun = _("patch file ${FILE}");
        params.file = file->localpath;
        add_result(ri, &params);
        free(params.msg);
        params.msg = NULL;

        reported = true;
    }

    /*
     * Collect patch stats and report based on thresholds.
     */
    ps = get_patch_stats(after_patch);
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.verb = VERB_CHANGED;
    params.noun = _("patch changes ${FILE}");
    params.file = file->localpath;

    if (ps.files == 0 && ps.lines > 0) {
        xasprintf(&params.msg, _("%s touches as many as %ld line%s"), file->localpath, ps.lines, (ps.lines > 1) ? "s" : "");
    } else if (ps.files > 0 && ps.lines == 0) {
        xasprintf(&params.msg, _("%s touches %ld file%s"), file->localpath, ps.files, (ps.files > 1) ? "s" : "");
    } else {
        xasprintf(&params.msg, _("%s touches %ld file%s and %s%ld line%s"), file->localpath, ps.files, (ps.files > 1) ? "s" : "", (ps.lines > 1) ? _("as many as ") : "", ps.lines, (ps.lines > 1) ? "s" : "");
    }

    add_result(ri, &params);
    free(params.msg);
    params.msg = NULL;

    reported = true;

    /* clean up */
    free(before_patch);
    free(after_patch);

    return true;
}

/*
 * Main driver for the 'patches' inspection.
 */
bool inspect_patches(struct rpminspect *ri)
{
    bool result = true;
    bool have_source = false;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;

    assert(ri != NULL);

    init_result_params(&params);
    params.header = NAME_PATCHES;

    /* Check for source package */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (headerIsSource(peer->after_hdr)) {
            have_source = true;

            if (headerIsSource(peer->before_hdr)) {
                comparison = true;
            }

            break;
        }
    }

    /* If no source found, we are not looking at source packages */
    if (!have_source) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        xasprintf(&params.msg, _("No source packages available, skipping inspection."));
        add_result(ri, &params);
        free(params.msg);
        reported = true;
        return result;
    }

    /* Set default parameters */
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.remedy = NULL;

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
            if (!patches_driver(ri, file)) {
                result = !(params.severity >= RESULT_VERIFY);
            }
        }

        /* Report any removed patch files from the SRPM */
        if (peer->before_files) {
            TAILQ_FOREACH(file, peer->before_files, items) {
                if (file->peer_file == NULL) {
                    xasprintf(&params.msg, _("Patch file `%s` removed"), file->localpath);
                    add_result(ri, &params);
                    free(params.msg);
                    reported = true;
                    result = !(params.severity >= RESULT_VERIFY);
                }
            }
        }
    }

    /* Sound the everything-is-ok alarm if everything is, in fact, ok */
    if (result && !reported) {
        init_result_params(&params);
        params.header = NAME_PATCHES;
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        add_result(ri, &params);
    }

    /* Our static list of PatchN: spec file members, dump it */
    list_free(patches, free);

    return result;
}
