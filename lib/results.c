/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
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

#include <sys/queue.h>
#include <assert.h>

#include "rpminspect.h"

/*
 * Initialize a struct result_params.
 */
void init_result_params(struct result_params *params)
{
    assert(params != NULL);

    params->severity = RESULT_OK;
    params->waiverauth = NOT_WAIVABLE;
    params->header = NULL;
    params->msg = NULL;
    params->details = NULL;
    params->remedy = NULL;
    params->verb = VERB_NIL;
    params->noun = NULL;
    params->arch = NULL;
    params->file = NULL;

    return;
}

/*
 * Initialize a new results_t list.
 */
results_t *init_results(void) {
    results_t *results = NULL;

    results = calloc(1, sizeof(*results));
    assert(results != NULL);
    TAILQ_INIT(results);
    return results;
}

/*
 * Free memory associated with an results_t list.
 */
void free_results(results_t *results) {
    results_entry_t *entry = NULL;

    if (results == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(results)) {
        entry = TAILQ_FIRST(results);
        TAILQ_REMOVE(results, entry, items);
        free(entry->msg);
        entry->msg = NULL;
        free(entry->details);
        entry->details = NULL;
        free(entry->noun);
        entry->noun = NULL;
        free(entry->arch);
        entry->arch = NULL;
        free(entry->file);
        entry->file = NULL;

        /* these are all consts */
        entry->header = NULL;
        entry->remedy = NULL;

        free(entry);
        entry = NULL;
    }

    free(results);
    results = NULL;

    return;
}

/*
 * Add the specified result to the list of results.  The parameters are the
 * members of the results_entry_t struct.  severity, waiverauth, header, and
 * msg are required.
 *
 * Strings passed in become results_entry_t members and will be freed when
 * the results_t is freed later.  Keep that in mind when building and
 * passing strings to this function.
 *
 * Pass NULL for any optional strings that you have no data for.
 */
void add_result_entry(results_t **results, struct result_params *params)
{
    results_entry_t *entry = NULL;

    assert(params != NULL);
    assert(params->severity >= 0);
    assert(params->header != NULL);

    if (*results == NULL) {
        *results = init_results();
    }

    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);

    entry->severity = params->severity;
    entry->waiverauth = params->waiverauth;
    entry->header = params->header;

    if (params->msg != NULL) {
        entry->msg = strdup(params->msg);
    }

    if (params->details != NULL) {
        entry->details = strdup(params->details);
    }

    if (params->remedy != NULL) {
        entry->remedy = params->remedy;
    }

    entry->verb = params->verb;

    if (params->noun != NULL) {
        entry->noun = strdup(params->noun);
    }

    if (params->arch != NULL) {
        entry->arch = strdup(params->arch);
    }

    if (params->file != NULL) {
        entry->file = strdup(params->file);
    }

    TAILQ_INSERT_TAIL(*results, entry, items);
    return;
}

/*
 * Shortcut to call add_result_entry() by giving the struct rpminspect.
 */
void add_result(struct rpminspect *ri, struct result_params *params)
{
    assert(ri != NULL);
    assert(params != NULL);
    assert(params->severity >= 0);

    if (params->severity > ri->worst_result) {
        ri->worst_result = params->severity;
    }

    add_result_entry(&ri->results, params);
    return;
}
