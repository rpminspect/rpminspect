/*
 * Copyright © 2020 Red Hat, Inc.
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
#include <assert.h>
#include <rpm/header.h>

#include "rpminspect.h"

/*
 * Main driver for the 'arch' inspection.
 */
bool inspect_arch(struct rpminspect *ri) {
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    const char *before_arch = NULL;
    const char *after_arch = NULL;
    string_list_t *before_arches = NULL;
    string_list_t *after_arches = NULL;
    string_entry_t *entry = NULL;
    string_list_t *lost = NULL;
    string_list_t *gain = NULL;
    struct result_params params;

    assert(ri != NULL);

    before_arches = calloc(1, sizeof(*before_arches));
    assert(before_arches != NULL);
    TAILQ_INIT(before_arches);

    after_arches = calloc(1, sizeof(*after_arches));
    assert(after_arches != NULL);
    TAILQ_INIT(after_arches);

    init_result_params(&params);
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = HEADER_ARCH;

    /* Gather up all the architectures */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (peer->before_hdr == NULL || peer->after_hdr == NULL) {
            /* missing peer packages; handled by other inspections */
            continue;
        }

        before_arch = headerGetString(peer->before_hdr, RPMTAG_ARCH);
        after_arch = headerGetString(peer->after_hdr, RPMTAG_ARCH);

        /*
         * loss of a noarch package is not something this inspection
         * needs to be concerned with.  this inspection checks for
         * loss of a target machine architecture as provided by the
         * before build
         */
        if (!strcmp(before_arch, "noarch") || !strcmp(after_arch, "noarch")) {
            continue;
        }

        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->data = strdup(before_arch);
        assert(entry->data != NULL);
        TAILQ_INSERT_TAIL(before_arches, entry, items);

        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->data = strdup(after_arch);
        assert(entry->data != NULL);
        TAILQ_INSERT_TAIL(after_arches, entry, items);
    }

    /* Compute what was lost and gained */
    lost = list_difference(before_arches, after_arches);
    gain = list_difference(after_arches, before_arches);

    /* Report results */
    if (lost != NULL && !TAILQ_EMPTY(lost)) {
        TAILQ_FOREACH(entry, lost, items) {
            params.severity = RESULT_VERIFY;
            params.remedy = REMEDY_ARCH_LOST;
            params.verb = VERB_REMOVED;
            params.noun = _("lost ${ARCH}");
            params.arch = entry->data;
            xasprintf(&params.msg, _("Architecture '%s' has disappeared"), entry->data);
            add_result(ri, &params);
            free(params.msg);
        }

        result = false;
    }


    if (gain != NULL && !TAILQ_EMPTY(gain)) {
        TAILQ_FOREACH(entry, gain, items) {
            params.severity = RESULT_INFO;
            params.remedy = REMEDY_ARCH_GAIN;
            params.verb = VERB_ADDED;
            params.noun = _("gained ${ARCH}");
            params.arch = entry->data;
            xasprintf(&params.msg, _("Architecture '%s' has appeared"), entry->data);
            add_result(ri, &params);
            free(params.msg);
        }

        result = false;
    }

    list_free(lost, NULL);
    list_free(gain, NULL);
    list_free(before_arches, free);
    list_free(after_arches, free);

    return result;
}
