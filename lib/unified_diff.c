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

/* Format the diff line to match 'diff -u' output */
static string_entry_t *format_line(const struct diff_ses ses)
{
    string_entry_t *entry = NULL;

    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    DEBUG_PRINT("ses.e=|%s|\n", *(const char **) ses.e);
    xasprintf(&entry->data, "%s%s", ses.type == DIFF_ADD ? "+" : ses.type == DIFF_DELETE ? "-" : " ", *(const char **) ses.e);
    return entry;
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
    string_list_t *context = NULL;
    string_entry_t *entry = NULL;
    bool inhunk = false;
    bool conthunk = false;
    struct unified_diff ud;
    size_t i = 1;
    size_t j = 1;

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

        /* determine what kind of line we're looking at */
        if (p.ses[i].type == DIFF_ADD || p.ses[i].type == DIFF_DELETE) {
            if (inhunk) {
                /* we're actively in a hunk, take the line */
                entry = format_line(p.ses[i]);
                DEBUG_PRINT("A: %ld=|%s|\n", i, entry->data);
                TAILQ_INSERT_TAIL(hunk, entry, items);
            } else {
                /* start a new hunk */
                hunk = calloc(1, sizeof(*hunk));
                assert(hunk != NULL);
                TAILQ_INIT(hunk);

                inhunk = true;

                /* take previous lines of context */
                if ((i - DIFF_CONTEXT_LINES) > 0) {
                    /* we have three possible previous lines of context */
                    j = i - DIFF_CONTEXT_LINES;

                    while (j <= 0) {
                        j++;
                    }
                }

                while (j < i) {
                    /* start a new context section */
                    if (context == NULL) {
                        context = calloc(1, sizeof(*context));
                        assert(context != NULL);
                        TAILQ_INIT(context);
                    }

                    /* add context line */
                    entry = format_line(p.ses[j]);
                    DEBUG_PRINT("B: %ld=|%s|\n", j, entry->data);
                    TAILQ_INSERT_TAIL(context, entry, items);
                    j++;
                }

                if (context) {
                    TAILQ_CONCAT(context, hunk, items);
                    hunk = context;
                    context = NULL;
                }

                /* we're actively in a hunk, take the line */
                entry = format_line(p.ses[i]);
                DEBUG_PRINT("C: %ld=|%s|\n", i, entry->data);
                TAILQ_INSERT_TAIL(hunk, entry, items);
            }
        } else if ((p.ses[i].type == DIFF_COMMON) && inhunk) {
            /* add the line */
            entry = format_line(p.ses[i]);
            DEBUG_PRINT("D: %ld=|%s|\n", i, entry->data);
            TAILQ_INSERT_TAIL(hunk, entry, items);

            /* another edit line within the context line region is part of this hunk */
            context = NULL;
            conthunk = false;
            j = i + 1;

            while (((j - i) < DIFF_CONTEXT_LINES) && (j < p.sessz)) {
                /* should we continue this hunk or not? */
                if (p.ses[j].type != DIFF_COMMON) {
                    conthunk = true;
                    break;
                }

                /* start a new context section */
                if (context == NULL) {
                    context = calloc(1, sizeof(*context));
                    assert(context != NULL);
                    TAILQ_INIT(context);
                }

                /* add context line */
                entry = format_line(p.ses[j]);
                DEBUG_PRINT("E: %ld=|%s|\n", j, entry->data);
                TAILQ_INSERT_TAIL(context, entry, items);
                j++;
            }

            /* add the context to the hunk */
            if (context && !TAILQ_EMPTY(context)) {
                TAILQ_CONCAT(hunk, context, items);
                context = NULL;
            }

            /* if we are still in the same hunk, continue */
            if (conthunk) {
                conthunk = false;
                continue;
            }

            /* generate the hunk header line and add it to the unified diff */
            if (hunk && !TAILQ_EMPTY(hunk)) {
                /* generate the hunk header line */
                entry = calloc(1, sizeof(*entry));
                assert(entry != NULL);
                xasprintf(&entry->data, "@@ -0,0 +0,0 @@\n");
                TAILQ_INSERT_HEAD(hunk, entry, items);
                TAILQ_CONCAT(unified, hunk, items);
            }

            /* close the hunk and continue */
            inhunk = false;
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

    /* split the strings */
    if (original == NULL) {
        orig = strsplit("", "\n");
    } else {
        orig = strsplit(original, "\n");
    }

    if (modified == NULL) {
        mod = strsplit("", "\n");
    } else {
        mod = strsplit(modified, "\n");
    }

    /* run the diff */
    unified = unified_output(orig, mod);

    /* cleanup */
    list_free(orig, free);
    list_free(mod, free);

    return unified;
}
