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

#include "rpminspect.h"

static bool _modularity_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    bool result = true;
    rpmTagType tt;
    rpmTagVal tv;
    char *modularitylabel = NULL;

    /* XXX
     * Some more should happen here, like determining if we are looking
     * at a module build.  Not all packages will be in modules, so there
     * needs to be an outer check to determine that first.
     */

    /* Find how to find the header */
    tv = rpmTagGetValue("modularitylabel");
    if (tv == -1) {
        /* XXX */
        return true;
    }

    tt = rpmTagGetTagType(tv);
    if (tt == RPM_NULL_TYPE) {
        /* XXX */
        return true;
    }

    /* Get the tag from the header */
    modularitylabel = headerGetAsString(file->rpm_header, tv);


printf("modularitylabel: |%s|\n", modularitylabel);

    free(modularitylabel);
    return result;
}

/*
 * Main driver for the 'modularity' inspection.
 */
bool inspect_modularity(struct rpminspect *ri) {
    bool result = false;

    assert(ri != NULL);
    result = foreach_peer_file(ri, _modularity_driver);

    if (result) {
        add_result(&ri->results, RESULT_OK, WAIVABLE_BY_ANYONE, HEADER_MODULARITY, NULL, NULL, NULL);
    }

    return result;
}
