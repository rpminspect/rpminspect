/*
 * Copyright (C) 2019  Red Hat, Inc.
 * Red Hat Author(s):  David Shea <dshea@redhat.com>
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

#include <stdbool.h>
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
        headerFree(entry->before_hdr);
        headerFree(entry->after_hdr);
        free(entry->before_rpm);
        entry->before_rpm = NULL;
        free(entry->after_rpm);
        entry->after_rpm = NULL;
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
void add_peer(rpmpeer_t **peers, int whichbuild, const char *pkg, Header *hdr) {
    rpmpeer_entry_t *peer = NULL;
    bool found = false;
    char *newname = NULL;
    char *newarch = NULL;
    char *existingname = NULL;
    char *existingarch = NULL;

    assert(peers != NULL);
    assert(pkg != NULL);
    assert(hdr != NULL);

    if (*peers == NULL) {
        *peers = init_rpmpeer();
    }

    /* Get the package or subpackage name and arch */
    newname = headerGetAsString(*hdr, RPMTAG_NAME);
    newarch = headerGetAsString(*hdr, RPMTAG_ARCH);

    /* First, see if we already have this peer */
    TAILQ_FOREACH(peer, *peers, items) {
        if (whichbuild == BEFORE_BUILD && peer->before_rpm != NULL) {
            existingname = peer->before_rpm;
        } else if (whichbuild == AFTER_BUILD && peer->after_rpm != NULL) {
            existingname = peer->after_rpm;
        }

        if (existingname != NULL && !strcmp(pkg, existingname)) {
            return;
        }
    }

    /* Second, if we don't have this peer, try to add it */
    existingname = NULL;

    TAILQ_FOREACH(peer, *peers, items) {
        if (whichbuild == BEFORE_BUILD && peer->after_rpm != NULL) {
            existingname = headerGetAsString(peer->after_hdr, RPMTAG_NAME);
            existingarch = headerGetAsString(peer->after_hdr, RPMTAG_ARCH);
        } else if (whichbuild == AFTER_BUILD && peer->before_rpm != NULL) {
            existingname = headerGetAsString(peer->before_hdr, RPMTAG_NAME);
            existingarch = headerGetAsString(peer->before_hdr, RPMTAG_ARCH);
        }

        if (existingname == NULL) {
            continue;
        }

        if (!strcmp(existingname, newname) && !strcmp(existingarch, newarch)) {
            /* found the existing peer */
            found = true;
            break;
        }
    }

    /* Add the peer if it doesn't already exist, otherwise add it */
    if (!found) {
        if ((peer = calloc(1, sizeof(*peer))) == NULL) {
            fprintf(stderr, "*** failed to allocate new peer peer\n");
            fflush(stderr);
            return;
        }
    }

    if (whichbuild == BEFORE_BUILD) {
        peer->before_hdr = headerCopy(*hdr);
        peer->before_rpm = strdup(pkg);
        peer->before_files = extract_rpm(pkg, *hdr);
    } else if (whichbuild == AFTER_BUILD) {
        peer->after_hdr = headerCopy(*hdr);
        peer->after_rpm = strdup(pkg);
        peer->after_files = extract_rpm(pkg, *hdr);
    }

    if (!found) {
        TAILQ_INSERT_TAIL(*peers, peer, items);
    }

    return;
}
