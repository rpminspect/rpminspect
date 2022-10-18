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
#include <stdbool.h>
#include <assert.h>

#include "rpminspect.h"

/*
 * Initialize the 'arches' list if the user did not specify it.
 * Contains a list of all architectures represented by the builds
 * specified.
 */
void init_arches(struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;
    bool found = false;
    const char *arch = NULL;

    assert(ri != NULL);

    if (ri->arches) {
        return;
    }

    ri->arches = calloc(1, sizeof(*ri->arches));
    assert(ri->arches != NULL);
    TAILQ_INIT(ri->arches);

    TAILQ_FOREACH(peer, ri->peers, items) {
        if (!peer->after_hdr) {
            continue;
        }

        arch = get_rpm_header_arch(peer->after_hdr);

        if (arch == NULL) {
            continue;
        }

        found = list_contains(ri->arches, arch);

        if (!found) {
            ri->arches = list_add(ri->arches, arch);
        }
    }

    return;
}

/*
 * Checks an RPM architecture against the user-specified list.
 * If the user did not specify a list of architectures, return
 * true.  If the user specified a list, only return true if the
 * RPM architecture is in the specified list.  The function
 * returns false if the architecture is not allowed.
 */
bool allowed_arch(const struct rpminspect *ri, const char *rpmarch)
{
    assert(ri != NULL);
    assert(rpmarch != NULL);

    if (ri->arches == NULL) {
        return true;
    }

    return list_contains(ri->arches, rpmarch);
}
