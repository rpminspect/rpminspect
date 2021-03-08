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
 * Main driver for the 'subpackages' inspection.
 */
bool inspect_subpackages(struct rpminspect *ri) {
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    string_list_t *before_pkgs = NULL;
    string_list_t *after_pkgs = NULL;
    string_entry_t *entry = NULL;
    string_list_t *lost = NULL;
    string_list_t *gain = NULL;
    char *arch = NULL;
    struct result_params params;

    assert(ri != NULL);

    before_pkgs = calloc(1, sizeof(*before_pkgs));
    assert(before_pkgs != NULL);
    TAILQ_INIT(before_pkgs);

    after_pkgs = calloc(1, sizeof(*after_pkgs));
    assert(after_pkgs != NULL);
    TAILQ_INIT(after_pkgs);

    /* Gather up all the package names */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (peer->before_hdr) {
            entry = calloc(1, sizeof(*entry));
            xasprintf(&entry->data, "%s %s", headerGetString(peer->before_hdr, RPMTAG_NAME), headerGetString(peer->before_hdr, RPMTAG_ARCH));
            TAILQ_INSERT_TAIL(before_pkgs, entry, items);
        }

        if (peer->after_hdr) {
            entry = calloc(1, sizeof(*entry));
            xasprintf(&entry->data, "%s %s", headerGetString(peer->after_hdr, RPMTAG_NAME), headerGetString(peer->after_hdr, RPMTAG_ARCH));
            TAILQ_INSERT_TAIL(after_pkgs, entry, items);
        }
    }

    /* Compute what was lost and gained */
    lost = list_difference(before_pkgs, after_pkgs);
    gain = list_difference(after_pkgs, before_pkgs);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = HEADER_SUBPACKAGES;

    /* Report results */
    if (lost != NULL && !TAILQ_EMPTY(lost)) {
        TAILQ_FOREACH(entry, lost, items) {
            arch = strstr(entry->data, " ");
            *arch = '\0';
            arch++;

            xasprintf(&params.msg, _("Subpackage '%s' has disappeared on '%s'"), entry->data, arch);
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.remedy = REMEDY_SUBPACKAGES_LOST;
            params.arch = arch;
            params.file = entry->data;
            params.verb = VERB_REMOVED;
            params.noun = _("subpackage ${FILE}");
            add_result(ri, &params);
            free(params.msg);
        }

        result = false;
    }


    if (gain != NULL && !TAILQ_EMPTY(gain)) {
        TAILQ_FOREACH(entry, gain, items) {
            arch = strstr(entry->data, " ");
            *arch = '\0';
            arch++;

            xasprintf(&params.msg, _("Subpackage '%s' has appeared on '%s'"), entry->data, arch);
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
            params.remedy = REMEDY_SUBPACKAGES_GAIN;
            params.arch = arch;
            params.file = entry->data;
            params.verb = VERB_ADDED;
            params.noun = _("subpackage ${FILE}");
            add_result(ri, &params);
            free(params.msg);
        }

        result = false;
    }

    list_free(lost, NULL);
    list_free(gain, NULL);
    list_free(before_pkgs, free);
    list_free(after_pkgs, free);

    return result;
}
