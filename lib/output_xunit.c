/*
 * Copyright © 2021 Red Hat, Inc.
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
#include <err.h>
#include <assert.h>

#include "rpminspect.h"

/*
 * Output a results_t in XUnit format.  This can be consumed by Jenkins or
 * other services that can read in XUnit data (e.g., GitHub).
 */
void output_xunit(const results_t *results, const char *dest, const severity_t threshold, __attribute__((unused)) const severity_t suppress)
{
    results_entry_t *result = NULL;
    int r = 0;
    int count = 0;
    int total = 0;
    int failures = 0;
    FILE *fp = NULL;
    const char *header = NULL;
    char *msg = NULL;
    char *rawcdata = NULL;
    char *cdata = NULL;

    /* count up total test cases and total failures */
    TAILQ_FOREACH(result, results, items) {
        if (header == NULL || strcmp(header, result->header)) {
            total++;
        }

        header = result->header;
    }

    /* default to stdout unless a filename was specified */
    if (dest == NULL) {
        fp = stdout;
    } else {
        fp = fopen(dest, "w");

        if (fp == NULL) {
            warn(_("opening %s for writing"), dest);
            return;
        }
    }

    header = NULL;
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp, "<testsuite tests=\"%d\" failures=\"%d\" errors=\"0\" skipped=\"0\">\n", total, failures);

    /* output the results */
    TAILQ_FOREACH(result, results, items) {
        if (header == NULL || strcmp(header, result->header)) {
            if (header != NULL) {
                fprintf(fp, "    </testcase>\n");
            }

            fprintf(fp, "    <testcase name=\"/%s\" classname=\"rpminspect\">\n", result->header);
            header = result->header;
            count = 1;
        }

        if (result->severity >= threshold) {
            fprintf(fp, "        <failure message=\"%s\">%s</failure>\n", result->msg, inspection_header_to_desc(result->header));
        }

        /* prepare the system out message */
        if (result->msg != NULL) {
            xasprintf(&msg, "%d) %s\n\n", count++, result->msg);
        }

        xasprintf(&rawcdata, _("Result: %s\nWaiver Authorization: %s\n\n"), strseverity(result->severity), strwaiverauth(result->waiverauth));
        msg = strappend(msg, rawcdata, NULL);
        free(rawcdata);

        if (result->details != NULL) {
            xasprintf(&rawcdata, _("Details:\n%s\n\n"), result->details);
            msg = strappend(msg, rawcdata, NULL);
            free(rawcdata);
        }

        if (result->remedy != NULL) {
            xasprintf(&rawcdata, _("Suggested Remedy:\n%s"), result->remedy);
            msg = strappend(msg, rawcdata, NULL);
            free(rawcdata);
        }

        /* escape the string for XML CDATA use */
        cdata = strxmlescape(msg);
        fprintf(fp, "        <system-out><![CDATA[%s]]></system-out>\n", cdata);
        free(cdata);
        free(msg);
    }

    /* tidy up and return */
    if (header != NULL) {
        fprintf(fp, "    </testcase>\n");
    }

    fprintf(fp, "</testsuite>\n");
    r = fflush(fp);
    assert(r == 0);

    if (dest != NULL) {
        r = fclose(fp);
        assert(r == 0);
    }

    return;
}
