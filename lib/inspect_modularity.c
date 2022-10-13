/*
 * Copyright 2019 Red Hat, Inc.
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

static bool modularity_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    rpmTagType tt;
    rpmTagVal tv;
    const char *modularitylabel = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Set up the result parameters */
    init_result_params(&params);
    params.severity = RESULT_BAD;
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_MODULARITY;
    params.remedy = REMEDY_MODULARITY;

    /* Build the message we'll use for errors */
    xasprintf(&params.msg, _("Package \"%s\" is part of a module but lacks the '%%{modularitylabel}' header tag."), headerGetString(file->rpm_header, RPMTAG_NAME));

    /* Find how to find the header */
    tv = rpmTagGetValue("modularitylabel");
    if (tv == -1) {
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    tt = rpmTagGetTagType(tv);
    if (tt == RPM_NULL_TYPE) {
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    /* Get the tag from the header */
    modularitylabel = headerGetString(file->rpm_header, tv);

    if (modularitylabel == NULL) {
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    free(params.msg);
    return result;
}

/*
 * Main driver for the 'modularity' inspection.
 */
bool inspect_modularity(struct rpminspect *ri)
{
    bool result = false;
    struct result_params params;

    assert(ri != NULL);

    if (ri->buildtype != KOJI_BUILD_MODULE) {
        init_result_params(&params);
        xasprintf(&params.msg, _("Inspection skipped because this build's type is not `module'."));
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_MODULARITY;
        add_result(ri, &params);
        free(params.msg);
        return true;
    }

    result = foreach_peer_file(ri, NAME_MODULARITY, modularity_driver);

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_MODULARITY;
        add_result(ri, &params);
    }

    return result;
}
