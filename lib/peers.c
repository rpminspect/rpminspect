/*
 * Copyright © 2019 Red Hat, Inc.
 * Author(s): David Shea <dshea@redhat.com>
 *            David Cantrell <dcantrell@redhat.com>
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

#include <stdbool.h>
#include <assert.h>
#include <err.h>

#include "rpminspect.h"

/*
 * Initialize a new rpmpeer_t list.
 */
rpmpeer_t *init_rpmpeer(void) {
    rpmpeer_t *peers = NULL;

    peers = calloc(1, sizeof(*(peers)));
    assert(peers != NULL);
    TAILQ_INIT(peers);
    return peers;
}

/*
 * Free memory associated with an rpmpeer_t list.
 */
void free_rpmpeer(rpmpeer_t *peers) {
    rpmpeer_entry_t *entry = NULL;

    if (peers == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(peers)) {
        entry = TAILQ_FIRST(peers);
        TAILQ_REMOVE(peers, entry, items);
        entry->before_hdr = NULL;
        entry->after_hdr = NULL;
        free(entry->before_rpm);
        entry->before_rpm = NULL;
        free(entry->after_rpm);
        entry->after_rpm = NULL;
        free(entry->before_root);
        entry->before_root = NULL;
        free(entry->after_root);
        entry->after_root = NULL;
        free_files(entry->before_files);
        entry->before_files = NULL;
        free_files(entry->after_files);
        entry->after_files = NULL;
        free(entry);
    }

    free(peers);

    return;
}

/*
 * Add the specified package as a peer in the list of packages.
 */
void add_peer(rpmpeer_t **peers, int whichbuild, bool fetch_only, const char *pkg, Header hdr)
{
    rpmpeer_entry_t *peer = NULL;
    bool found = false;
    const char *newname = NULL;
    const char *newarch = NULL;
    const char *existingname = NULL;
    const char *existingarch = NULL;
    bool existingsrc = false;
    bool newsrc = false;

    assert(peers != NULL);
    assert(pkg != NULL);
    assert(hdr != NULL);

    if (*peers == NULL) {
        *peers = init_rpmpeer();
    }

    /* Get the package or subpackage name and arch */
    newname = headerGetString(hdr, RPMTAG_NAME);
    newarch = get_rpm_header_arch(hdr);
    newsrc = headerIsSource(hdr);

    /* If we don't have this peer, try to add it */
    TAILQ_FOREACH(peer, *peers, items) {
        if (whichbuild == BEFORE_BUILD && peer->after_rpm != NULL) {
            existingname = headerGetString(peer->after_hdr, RPMTAG_NAME);
            existingarch = get_rpm_header_arch(peer->after_hdr);
            existingsrc = headerIsSource(peer->after_hdr);
        } else if (whichbuild == AFTER_BUILD && peer->before_rpm != NULL) {
            existingname = headerGetString(peer->before_hdr, RPMTAG_NAME);
            existingarch = get_rpm_header_arch(peer->before_hdr);
            existingsrc = headerIsSource(peer->before_hdr);
        }

        if (existingname == NULL) {
            continue;
        }

        if ((existingsrc && newsrc && !strcmp(existingname, newname)) ||
            (!existingsrc && !newsrc && !strcmp(existingname, newname) && !strcmp(existingarch, newarch))) {
            /* found the existing peer */
            found = true;
            break;
        }
    }

    /* Add the peer if it doesn't already exist, otherwise add it */
    if (!found) {
        if ((peer = calloc(1, sizeof(*peer))) == NULL) {
            warn("calloc() new peer");
            return;
        }
    }

    if (whichbuild == BEFORE_BUILD) {
        peer->before_hdr = hdr;
        peer->before_rpm = strdup(pkg);

        if (fetch_only) {
            peer->before_files = NULL;
            peer->after_root = NULL;
        } else {
            peer->before_files = extract_rpm(pkg, hdr, &peer->before_root);
        }
    } else if (whichbuild == AFTER_BUILD) {
        peer->after_hdr = hdr;
        peer->after_rpm = strdup(pkg);

        if (fetch_only) {
            peer->after_files = NULL;
            peer->after_root = NULL;
        } else {
            peer->after_files = extract_rpm(pkg, hdr, &peer->after_root);
        }
    }

    if (!found) {
        TAILQ_INSERT_TAIL(*peers, peer, items);
    }

    if (peer->before_files && peer->after_files) {
        find_file_peers(peer->before_files, peer->after_files);
    }

    return;
}
