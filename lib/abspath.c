/*
 * Copyright (C) 2021 Red Hat, Inc.
 * Author(s): David Cantrell <dcantrell@redhat.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "rpminspect.h"

/*
 * Canonicalize a path with relative references.  Does not rely on
 * filesystem existence.  Caller must free the returned string.
 */
char *abspath(const char *path)
{
    char *r = NULL;
    char *p = NULL;
    string_list_t *tokens = NULL;
    string_list_t *newpath = NULL;
    string_entry_t *token = NULL;
    string_entry_t *element = NULL;

    if (path == NULL) {
        return NULL;
    }

    if (!strcmp(path, "")) {
        return strdup(path);
    }

    /* split path in to tokens */
    tokens = strsplit(path, "/");
    assert(tokens != NULL);

    /* our new path elements */
    newpath = calloc(1, sizeof(*newpath));
    assert(newpath != NULL);
    TAILQ_INIT(newpath);

    /* handle each part of the path */
    TAILQ_FOREACH(token, tokens, items) {
        if (!strcmp(token->data, "") || !strcmp(token->data, ".") || (!strcmp(token->data, "..") && TAILQ_EMPTY(newpath))) {
            /* no need to add this token */
            continue;
        } else if (!strcmp(token->data, "..") && !TAILQ_EMPTY(newpath)) {
            /* back up a path element */
            element = TAILQ_LAST(newpath, string_entry_s);
            TAILQ_REMOVE(newpath, element, items);
            free(element->data);
            free(element);
        } else {
            /* take this path element */
            element = calloc(1, sizeof(*element));
            assert(element != NULL);
            element->data = strdup(token->data);
            TAILQ_INSERT_TAIL(newpath, element, items);
        }
    }

    /* generate the final path string */
    p = list_to_string(newpath, "/");

    if (p) {
        xasprintf(&r, "/%s", p);
    } else {
        r = strdup("/");
    }

    /* clean up */
    list_free(tokens, free);
    list_free(newpath, free);
    free(p);

    return r;
}
