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

#include <stdlib.h>
#include <sys/queue.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

/* Comparison callback passed to diff() */
static int cmp(const void *p1, const void *p2)
{
    return !strcmp(*(const char **) p1, *(const char **) p2);
}

/*
 * Create the unified diff output as a string_list_t given the
 * result of diff().  Caller must free the result.
 */
static string_list_t *unified_output(const string_list_t *original, const string_list_t *modified)
{
    int rc = 0;
    struct diff p;
    string_list_t *unified = NULL;
    string_list_t *hunk = NULL;
    string_entry_t *entry = NULL;
    struct unified_diff ud;
    size_t i = 0;
    size_t j = 0;
    bool context = false;

    assert(original != NULL);
    assert(modified != NULL);

    /* run the diff */
    rc = diff(&p, cmp, sizeof(char *), list_to_array(original), list_len(original), list_to_array(modified), list_len(modified));

    if (rc < 0) {
        fprintf(stderr, "*** %s (%d): %s\n", __func__, __LINE__, strerror(errno));
        return NULL;
    }

    if (rc == 0) {
        fprintf(stderr, "*** cannot compute distance\n");
        return NULL;
    }

    /* generated unified diff output */
    unified = calloc(1, sizeof(*unified));
    assert(unified != NULL);
    TAILQ_INIT(unified);

    memset(&ud, 0, sizeof(ud));

    for (i = 0; i < p.sessz; i++) {
        if (hunk == NULL) {
            hunk = calloc(1, sizeof(*hunk));
            assert(hunk != NULL);
            TAILQ_INIT(hunk);
        }

        if (p.ses[i].type == DIFF_ADD || p.ses[i].type == DIFF_DELETE) {
            if (!context) {
                /* start a context marker */
                context = true;
                ud.from = p.ses[i].originIdx - DIFF_CONTEXT_LINES;

                if (ud.to == 0) {
                    ud.to = ud.from;
                }

                for (j = DIFF_CONTEXT_LINES; (j > 0) && ((i - j) >= 1); j--) {
                    entry = calloc(1, sizeof(*entry));
                    assert(entry != NULL);
                    xasprintf(&entry->data, " %s", *(const char **) p.ses[i - j].e);
                    TAILQ_INSERT_TAIL(hunk, entry, items);
                    ud.fromlen++;
                    ud.tolen++;
                }
            }

            if (p.ses[i].type == DIFF_ADD) {
                ud.symbol = "+";
                ud.to++;
                ud.tolen++;
            } else if (p.ses[i].type == DIFF_DELETE) {
                ud.symbol = "-";
                ud.to--;
            } else {
                ud.symbol = " ";
            }

            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            xasprintf(&entry->data, "%s%s", ud.symbol, *(const char **) p.ses[i].e);
            TAILQ_INSERT_TAIL(hunk, entry, items);
            ud.fromlen++;
        } else {
            if (context) {
                /* end the context marker */
                context = false;

                for (j = 1; (j <= DIFF_CONTEXT_LINES) && ((i + j) < p.sessz); j++) {
                    entry = calloc(1, sizeof(*entry));
                    assert(entry != NULL);
                    xasprintf(&entry->data, " %s", *(const char **) p.ses[i + j].e);
                    TAILQ_INSERT_TAIL(hunk, entry, items);
                    ud.fromlen++;
                    ud.tolen++;
                }

                /* add this hunk to the unified diff */
                entry = calloc(1, sizeof(*entry));
                assert(entry != NULL);
                xasprintf(&entry->data, "@@ -%lu,%lu +%lu,%lu @@\n", ud.from, ud.fromlen, ud.to, ud.tolen);
                TAILQ_INSERT_TAIL(unified, entry, items);
                TAILQ_CONCAT(unified, hunk, items);

                /* start next hunk */
                hunk = NULL;
                ud.from = 0;
                ud.fromlen = 0;
                ud.to = 0;
                ud.tolen = 0;
            }
        }
    }

    /* cleanup */
    list_free(hunk, NULL);
    free(p.ses);
    free(p.lcs);

    return unified;
}

/*
 * Given two files, construct a string_list_t that is unified diff
 * output (diff -u) of the two files.  NULL means they are the same.
 * Caller must free the returned string_list_t.
 */
string_list_t *unified_file_diff(const char *original, const char *modified)
{
    char *origfile = NULL;
    char *modfile = NULL;
    string_list_t *orig = NULL;
    string_list_t *mod = NULL;
    string_list_t *unified = NULL;

    assert(original != NULL);
    assert(modified != NULL);

    /* get full real paths to the files */
    origfile = realpath(original, NULL);
    assert(origfile != NULL);
    modfile = realpath(modified, NULL);
    assert(modfile != NULL);

    /* read in the source files */
    orig = read_file(origfile);
    mod = read_file(modfile);

    /* get unified output */
    unified = unified_output(orig, mod);

    /* clean up */
    list_free(orig, free);
    list_free(mod, free);
    free(origfile);
    free(modfile);

    return unified;
}

/*
 * Given two strings, construct a string_list_t that is unified diff
 * output (diff -u) of the two strings.  The strings are assumed to be
 * newline delimited.  This function will construct a character array
 * on that basis.  NULL means they are the same.  Caller must free the
 * returned string_list_t.
 */
string_list_t *unified_str_diff(const char *original, const char *modified)
{
    string_list_t *unified = NULL;
    string_list_t *orig = NULL;
    string_list_t *mod = NULL;

    assert(original != NULL);
    assert(modified != NULL);

    /* split the strings */
    orig = strsplit(original, "\n");
    mod = strsplit(modified, "\n");

    /* run the diff */
    unified = unified_output(orig, mod);

    /* cleanup */
    list_free(orig, free);
    list_free(mod, free);

    return unified;
}
