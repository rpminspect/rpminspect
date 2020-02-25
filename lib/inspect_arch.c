/*
 * Copyright (C) 2020  Red Hat, Inc.
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
#include <assert.h>
#include <rpm/header.h>

#include "rpminspect.h"

/*
 * Main driver for the 'arch' inspection.
 */
bool inspect_arch(struct rpminspect *ri) {
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    string_list_t *before_arches = NULL;
    string_list_t *after_arches = NULL;
    string_entry_t *entry = NULL;
    string_list_t *lost = NULL;
    string_list_t *gain = NULL;
    char *msg = NULL;

    assert(ri != NULL);

    before_arches = calloc(1, sizeof(*before_arches));
    assert(before_arches != NULL);
    TAILQ_INIT(before_arches);

    after_arches = calloc(1, sizeof(*after_arches));
    assert(after_arches != NULL);
    TAILQ_INIT(after_arches);

    /* Gather up all the architectures */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (peer->before_hdr) {
            entry = calloc(1, sizeof(*entry));
            entry->data = strdup(headerGetString(peer->before_hdr, RPMTAG_ARCH));
            TAILQ_INSERT_TAIL(before_arches, entry, items);
        }

        if (peer->after_hdr) {
            entry = calloc(1, sizeof(*entry));
            entry->data = strdup(headerGetString(peer->after_hdr, RPMTAG_ARCH));
            TAILQ_INSERT_TAIL(after_arches, entry, items);
        }
    }

    /* Compute what was lost and gained */
    lost = list_difference(before_arches, after_arches);
    gain = list_difference(after_arches, before_arches);

    /* Report results */
    if (lost != NULL && !TAILQ_EMPTY(lost)) {
        TAILQ_FOREACH(entry, lost, items) {
            xasprintf(&msg, "Architecture '%s' has disappeared", entry->data);
            add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_ARCH, msg, NULL, REMEDY_ARCH_LOST);
            free(msg);
        }

        list_free(lost, free);
        result = false;
    }


    if (gain != NULL && !TAILQ_EMPTY(gain)) {
        TAILQ_FOREACH(entry, gain, items) {
            xasprintf(&msg, "Architecture '%s' has appeared", entry->data);
            add_result(ri, RESULT_INFO, WAIVABLE_BY_ANYONE, HEADER_ARCH, msg, NULL, REMEDY_ARCH_GAIN);
            free(msg);
        }

        list_free(gain, free);
        result = false;
    }

    list_free(before_arches, free);
    list_free(after_arches, free);

    return result;
}
