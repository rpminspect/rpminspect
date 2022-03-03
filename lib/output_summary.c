/*
 * Copyright 2021 Red Hat, Inc.
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
#include <string.h>
#include <errno.h>
#include <err.h>
#include <assert.h>

#include "rpminspect.h"

/*
 * Output a results_t in summary text format.
 */
void output_summary(const results_t *results, const char *dest, __attribute__((unused)) const severity_t threshold, const severity_t suppress)
{
    results_entry_t *result = NULL;
    int r = 0;
    FILE *fp = NULL;
    char *verb = NULL;
    char *msg = NULL;
    char *tmp = NULL;
    size_t width = tty_width();

    /* default to stdout unless a filename was specified */
    if (dest == NULL) {
        fp = stdout;
    } else {
        fp = fopen(dest, "w");

        if (fp == NULL) {
            warn(_("error opening %s for writing"), dest);
            return;
        }
    }

    /* output the results */
    TAILQ_FOREACH(result, results, items) {
        /* skip conditions */
        if (!strcmp(result->header, NAME_DIAGNOSTICS)
            || (result->verb == VERB_OK && result->noun == NULL)
            || (result->severity >= suppress)) {
            continue;
        }

        /* get a string representing the verb */
        if (result->verb == VERB_ADDED) {
            verb = _("added");
        } else if (result->verb == VERB_REMOVED) {
            verb = _("removed");
        } else if (result->verb == VERB_CHANGED) {
            verb = _("changed");
        } else if (result->verb == VERB_FAILED) {
            verb = _("FAILED");
        } else if (result->verb == VERB_OK) {
            verb = _("ok");
        } else {
            verb = _("unknown");
        }

        /* construct the basic message */
        xasprintf(&msg, "%-12s %s (%s)\n", verb, result->noun, result->header);

        /* replace ${FILE} */
        if (strstr(msg, "${FILE}") && result->file != NULL) {
            tmp = strreplace(msg, "${FILE}", result->file);
            free(msg);
            msg = tmp;
        }

        /* replace ${ARCH} */
        if (strstr(msg, "${ARCH}") && result->arch != NULL) {
            tmp = strreplace(msg, "${ARCH}", result->arch);
            free(msg);
            msg = tmp;
        }

        /* print the result */
        if (width) {
            printwrap(msg, width, 0, fp);
        } else {
            fprintf(fp, "%s", msg);
        }

        /* clean up */
        free(msg);
    }

    /* tidy up and return */
    r = fflush(fp);
    assert(r == 0);

    if (dest != NULL) {
        r = fclose(fp);
        assert(r == 0);
    }

    return;
}
