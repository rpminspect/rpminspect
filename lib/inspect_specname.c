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
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool specgood = false;
static bool seen = false;

static bool specname_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    char *specfile = NULL;
    char *dot = NULL;
    char *desc = NULL;
    char *primary = NULL;
    struct result_params params;

    /* Skip binary packages */
    if (!headerIsSource(file->rpm_header)) {
        return true;
    }

    /* We only want to look at the spec file */
    if (!strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
        return true;
    }

    /* Spec files are all named in a standard way */
    if (ri->specprimary == PRIMARY_NAME) {
        primary = strdup(headerGetString(file->rpm_header, RPMTAG_NAME));
    } else if (ri->specprimary == PRIMARY_FILENAME) {
        primary = strdup(file->localpath);
        dot = rindex(primary, '.');
        assert(dot != NULL);
        *dot = '\0';
    }

    xasprintf(&specfile, "%s%s", primary, SPEC_FILENAME_EXTENSION);

    /* Match spec file name per conf file rule */
    if (ri->specmatch == MATCH_FULL && !strcmp(file->localpath, specfile)) {
        specgood = true;
    } else if (ri->specmatch == MATCH_PREFIX && strprefix(file->localpath, primary)) {
        specgood = true;
    } else if (ri->specmatch == MATCH_SUFFIX && strsuffix(file->localpath, specfile)) {
        specgood = true;
    }

    /*
     * Emit a failure if we're looking at what we think is a spec file
     * but it's not named in the expected way.
     */
    if (!specgood) {
        /* Set up result parameters */
        init_result_params(&params);
        params.severity = RESULT_BAD;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_SPECNAME;
        params.remedy = REMEDY_SPECNAME;
        params.file = file->localpath;

        if (ri->specmatch == MATCH_FULL) {
            desc = _("exactly match");
        } else if (ri->specmatch == MATCH_PREFIX) {
            desc = _("begin with");
        } else if (ri->specmatch == MATCH_SUFFIX) {
            desc = _("end with");
        }

        xasprintf(&params.msg, _("Spec filename does not %s the primary name %s; got '%s'"), desc, primary, file->localpath);
        add_result(ri, &params);
        free(params.msg);
    }

    free(primary);
    free(specfile);
    seen = true;
    return specgood;
}

/*
 * Main driver for the 'specname' inspection.
 */
bool inspect_specname(struct rpminspect *ri) {
    struct result_params params;

    assert(ri != NULL);
    foreach_peer_file(ri, NAME_SPECNAME, specname_driver, false);

    init_result_params(&params);
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_SPECNAME;

    if (specgood) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    } else if (!seen) {
        params.severity = RESULT_INFO;
        xasprintf(&params.msg, _("The specname inspection is only for source packages, skipping."));
        add_result(ri, &params);
        free(params.msg);

        /*
         * There's no reason to fail this test for an informational message.
         */
        specgood = true;
    }

    return specgood;
}
