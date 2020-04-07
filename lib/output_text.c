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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

/*
 * Output a results_t in plain text format.
 */
void output_text(const results_t *results, const char *dest) {
    results_entry_t *result = NULL;
    int r = 0;
    int count = 0;
    int len = 0;
    bool displayed_header = false;
    bool first = true;
    FILE *fp = NULL;
    char *header = NULL;
    char *msg = NULL;
    size_t width = tty_width();

    /* default to stdout unless a filename was specified */
    if (dest == NULL) {
        fp = stdout;
    } else {
        fp = fopen(dest, "w");

        if (fp == NULL) {
            fprintf(stderr, _("*** Error opening %s for writing: %s\n"), dest, strerror(errno));
            fflush(stderr);
            return;
        }
    }

    /* output the results */
    TAILQ_FOREACH(result, results, items) {
        if (header == NULL || strcmp(header, result->header)) {
            header = result->header;
            displayed_header = false;
            count = 1;
        }

        if (first) {
            first = false;
        } else {
            fprintf(fp, "\n");
        }

        if (!displayed_header) {
            len = strlen(header) + 1;
            fprintf(fp, "%s:\n", header);

            for (r = 0; r < len; r++) {
                fprintf(fp, "-");
            }

            fprintf(fp, "\n");
            displayed_header = true;
        }

        if (result->msg != NULL) {
            xasprintf(&msg, "%d) %s\n", count++, result->msg);

            if (width) {
                printwrap(msg, width, 0, fp);
            } else {
                fprintf(fp, "%s", msg);
            }

            fprintf(fp, "\n");
            free(msg);
        }

        fprintf(fp, _("Result: %s\n"), strseverity(result->severity));

        fprintf(fp, _("Waiver Authorization: %s\n\n"), strwaiverauth(result->waiverauth));

        if (result->details != NULL) {
            fprintf(fp, _("Details:\n%s\n\n"), result->details);
        }

        if (result->remedy != NULL) {
            xasprintf(&msg, _("Suggested Remedy:\n%s"), result->remedy);

            if (width) {
                printwrap(msg, width, 0, fp);
            } else {
                fprintf(fp, "%s", msg);
            }

            free(msg);
        }

        fprintf(fp, "\n");
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
