/*
 * Copyright (C) 2020  Red Hat, Inc.
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
    string_entry_t *entry = NULL;
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

        found = false;

        TAILQ_FOREACH(entry, ri->arches, items) {
            if (!strcmp(arch, entry->data)) {
                found = true;
                break;
            }
        }

        if (!found) {
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->data = strdup(arch);
            TAILQ_INSERT_TAIL(ri->arches, entry, items);
        }
    }

    return;
}
