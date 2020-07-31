/*
 * Copyright (C) 2020  Red Hat, Inc.
 * Red Hat Author(s):  David Cantrell <dcantrell@redhat.com>
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

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <glob.h>
#include <assert.h>
#include <err.h>

#ifdef _COMPAT_QUEUE
#include "compat/queue.h"
#else
#include <sys/queue.h>
#endif

#include "rpminspect.h"

/**
 * @file ignore.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2020
 * @brief Functions for handling the 'ignores' from the config file.
 * @copyright LGPL-3.0-or-later
 */

/**
 * @brief Given a path and struct rpminspect, determine if the path should be ignored or not.
 *
 * @param ri The struct rpminspect for the program.
 * @param path The relative path to check (i.e., localpath).
 * @param root The root directory, optional (pass NULL to use '/').
 * @return True if path should be ignored, false otherwise.
 */
bool ignore_path(const struct rpminspect *ri, const char *path, const char *root)
{
    bool match = false;
    string_entry_t *entry = NULL;
    int gflags = GLOB_NOSORT | GLOB_PERIOD | GLOB_BRACE;
    char *globpath = NULL;
    char *globsub = NULL;
    glob_t found;
    size_t len = 0;
    int r = 0;
    size_t i = 0;

    assert(ri != NULL);

    if (path == NULL) {
        return true;
    }

    if (root) {
        len = strlen(root);
    }

    TAILQ_FOREACH(entry, ri->ignores, items) {
        if (root == NULL) {
            globpath = strdup(entry->data);
        } else {
            xasprintf(&globpath, "%s%s", root, entry->data);
        }

        r = glob(globpath, gflags, NULL, &found);

        if (r == GLOB_NOSPACE || r == GLOB_ABORTED) {
            warn("%s: glob()", __func__);
        }

        if (r != 0) {
            free(globpath);
            continue;
        }

        for (i = 0; i < found.gl_pathc; i++) {
            globsub = found.gl_pathv[i] + len;

            if (!strcmp(globsub, path)) {
                match = true;
                break;
            }
        }

        free(globpath);
        globfree(&found);

        if (match) {
            break;
        }
    }

    return match;
}
