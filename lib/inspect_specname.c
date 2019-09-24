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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool specgood = false;
static bool seen = false;

static bool specname_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    char *specfile = NULL;
    char *msg = NULL;
    char *pkgname = NULL;

    /* Skip binary packages */
    if (!headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Spec files are all named in a standard way */
    pkgname = headerGetAsString(file->rpm_header, RPMTAG_NAME);
    xasprintf(&specfile, "%s%s", pkgname, SPEC_FILENAME_EXTENSION);

    /* We only want to look at the spec files */
    if (!strcmp(file->localpath, specfile)) {
        specgood = true;
    } else if (strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
        /*
         * Emit a failure if we're looking at what we think is a spec file
         * but it's not named in the expected way.
         */
        xasprintf(&msg, "Spec filename does not match the pattern of NAME%s; expected '%s', got '%s'", SPEC_FILENAME_EXTENSION, specfile, file->localpath);
        add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_SPECNAME, msg, NULL, REMEDY_SPECNAME);
        free(msg);
        specgood = false;
    }

    free(specfile);
    seen = true;
    return specgood;
}

/*
 * Main driver for the 'specname' inspection.
 */
bool inspect_specname(struct rpminspect *ri) {
    char *msg = NULL;

    assert(ri != NULL);
    foreach_peer_file(ri, specname_driver);

    if (specgood) {
        add_result(&ri->results, RESULT_OK, WAIVABLE_BY_ANYONE, HEADER_SPECNAME, NULL, NULL, NULL);
    } else if (!seen) {
        xasprintf(&msg, "The specname inspection is only for source packages.");
        add_result(&ri->results, RESULT_BAD, NOT_WAIVABLE, HEADER_SPECNAME, msg, NULL, NULL);
        free(msg);
    }

    return specgood;
}
