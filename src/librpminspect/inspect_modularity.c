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

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool modularity_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    bool result = true;
    rpmTagType tt;
    rpmTagVal tv;
    char *modularitylabel = NULL;
    char *msg = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* Build the message we'll use for errors */
    xasprintf(&msg, "Package \"%s\" is part of a module but lacks the '%%{modularitylabel}' header tag.", headerGetAsString(file->rpm_header, RPMTAG_NAME));

    /* Find how to find the header */
    tv = rpmTagGetValue("modularitylabel");
    if (tv == -1) {
        add_result(&ri->results, RESULT_BAD, NOT_WAIVABLE, HEADER_MODULARITY, msg, NULL, REMEDY_MODULARITY);
        free(msg);
        return false;
    }

    tt = rpmTagGetTagType(tv);
    if (tt == RPM_NULL_TYPE) {
        add_result(&ri->results, RESULT_BAD, NOT_WAIVABLE, HEADER_MODULARITY, msg, NULL, REMEDY_MODULARITY);
        free(msg);
        return false;
    }

    /* Get the tag from the header */
    modularitylabel = headerGetAsString(file->rpm_header, tv);

    if (modularitylabel == NULL) {
        add_result(&ri->results, RESULT_BAD, NOT_WAIVABLE, HEADER_MODULARITY, msg, NULL, REMEDY_MODULARITY);
        free(msg);
        return false;
    }

    free(msg);
    free(modularitylabel);
    return result;
}

/*
 * Main driver for the 'modularity' inspection.
 */
bool inspect_modularity(struct rpminspect *ri) {
    bool result = false;

    if (ri->buildtype != KOJI_BUILD_MODULE) {
        return true;
    }

    assert(ri != NULL);
    result = foreach_peer_file(ri, modularity_driver);

    if (result) {
        add_result(&ri->results, RESULT_OK, WAIVABLE_BY_ANYONE, HEADER_MODULARITY, NULL, NULL, NULL);
    }

    return result;
}
