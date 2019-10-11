/*
 * Copyright (C) 2019  Red Hat, Inc.
 * Red Hat Author(s):  David Shea <dshea@redhat.com>
 *                     David Cantrell <dcantrell@redhat.com>
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

#include <stdbool.h>
#include <assert.h>

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
int add_peer(rpmpeer_t **peers, int whichbuild, bool fetch_only, const char *pkg, Header *hdr) {
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
    newname = headerGetString(*hdr, RPMTAG_NAME);
    newarch = headerGetString(*hdr, RPMTAG_ARCH);
    newsrc = headerIsSource(*hdr);

    /* See if we already have this peer */
    TAILQ_FOREACH(peer, *peers, items) {
        existingname = NULL;
        existingarch = NULL;
        existingsrc = false;

        if (existingname != NULL && !strcmp(pkg, existingname)) {
            return 0;
        }
    }

    /* Second, if we don't have this peer, try to add it */
    existingname = NULL;

    TAILQ_FOREACH(peer, *peers, items) {
        if (whichbuild == BEFORE_BUILD && peer->after_rpm != NULL) {
            existingname = headerGetString(peer->after_hdr, RPMTAG_NAME);
            existingarch = headerGetString(peer->after_hdr, RPMTAG_ARCH);
            existingsrc = headerIsSource(peer->after_hdr);
        } else if (whichbuild == AFTER_BUILD && peer->before_rpm != NULL) {
            existingname = headerGetString(peer->before_hdr, RPMTAG_NAME);
            existingarch = headerGetString(peer->before_hdr, RPMTAG_ARCH);
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
            fprintf(stderr, "*** failed to allocate new peer peer\n");
            fflush(stderr);
            return 1;
        }
    }

    if (whichbuild == BEFORE_BUILD) {
        peer->before_hdr = headerCopy(*hdr);
        peer->before_rpm = strdup(pkg);

        if (fetch_only) {
            peer->before_files = NULL;
        } else {
            peer->before_files = extract_rpm(pkg, *hdr);
            if (peer->before_files == NULL) {
                goto fail;
            }
        }
    } else if (whichbuild == AFTER_BUILD) {
        peer->after_hdr = headerCopy(*hdr);
        peer->after_rpm = strdup(pkg);

        if (fetch_only) {
            peer->after_files = NULL;
        } else {
            peer->after_files = extract_rpm(pkg, *hdr);
            if (peer->after_files == NULL) {
                goto fail;
            }
        }
    }

    if (!found) {
        TAILQ_INSERT_TAIL(*peers, peer, items);
    }

    if (peer->before_files && peer->after_files) {
        find_file_peers(peer->before_files, peer->after_files);
    }

    return 0;

fail:
    free(peer->before_hdr);
    free(peer->before_rpm);
    free(peer);
    return 1;
}
