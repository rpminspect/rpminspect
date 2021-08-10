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

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <assert.h>
#include <err.h>
#include <glob.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>
#include "queue.h"
#include "rpminspect.h"

/**
 * @brief Return the before build debuginfo package path where the
 * package was extracted for rpminspect.  The path must match the
 * architecture provided.
 *
 * IMPORTANT: Do not free the returned string.
 *
 * @param ri The struct rpminspect for the program.
 * @param binarch The required debuginfo architecture.
 * @return Full path to the extract before build debuginfo package, or
 * NULL if not found.
 */
const char *get_before_debuginfo_path(struct rpminspect *ri, const char *binarch)
{
    rpmpeer_entry_t *peer = NULL;
    const char *name = NULL;
    const char *arch = NULL;

    assert(ri != NULL);
    assert(binarch != NULL);

    /* try to find a debuginfo package */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (peer->before_hdr == NULL) {
            continue;
        }

        arch = get_rpm_header_arch(peer->before_hdr);
        assert(arch != NULL);

        if (strcmp(arch, binarch)) {
            /* not the same architecture */
            continue;
        }

        name = headerGetString(peer->before_hdr, RPMTAG_NAME);
        assert(name != NULL);

        /* found the debuginfo package */
        if (strsuffix(name, DEBUGINFO_SUFFIX)) {
            /* just copy the pointer, no need to dupe it */
            return peer->before_root;
        }
    }

    return NULL;
}

/**
 * @brief Return the after build debuginfo package path where the
 * package was extracted for rpminspect.  The path must match the
 * architecture provided.
 *
 * IMPORTANT: Do not free the returned string.
 *
 * @param ri The struct rpminspect for the program.
 * @param binarch The required debuginfo architecture.
 * @return Full path to the extract after build debuginfo package, or
 * NULL if not found.
 */
const char *get_after_debuginfo_path(struct rpminspect *ri, const char *binarch)
{
    rpmpeer_entry_t *peer = NULL;
    const char *name = NULL;
    const char *arch = NULL;

    assert(ri != NULL);
    assert(binarch != NULL);

    /* try to find a debuginfo package */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (peer->after_hdr == NULL) {
            continue;
        }

        arch = get_rpm_header_arch(peer->after_hdr);
        assert(arch != NULL);

        if (strcmp(arch, binarch)) {
            /* not the same architecture */
            continue;
        }

        name = headerGetString(peer->after_hdr, RPMTAG_NAME);
        assert(name != NULL);

        /* found the debuginfo package */
        if (strsuffix(name, DEBUGINFO_SUFFIX)) {
            /* just copy the pointer, no need to dupe it */
            return peer->after_root;
        }
    }

    return NULL;
}

/*
 * Checks to see if the given path is a readable directory.
 */
bool usable_path(const char *path)
{
    struct stat sb;

    if (path == NULL) {
        return false;
    }

    if (access(path, R_OK) == -1) {
        return false;
    }

    memset(&sb, 0, sizeof(sb));

    if (lstat(path, &sb) == -1) {
        warn("lstat(%s)", path);
        return false;
    }

    if (!S_ISDIR(sb.st_mode)) {
        return false;
    }

    return true;
}

/*
 * Helper function for glob(7) matching given a path string and
 * (optional) root directory.
 */
bool match_path(const char *pattern, const char *root, const char *needle)
{
    bool match = false;
    int r = 0;
    int gflags = GLOB_NOSORT | GLOB_PERIOD;
    char globpath[PATH_MAX + 1];
    char *globsub = NULL;
    char *gp = globpath;
    glob_t found;
    size_t len = 0;
    size_t i = 0;
    char *n = NULL;

    assert(pattern != NULL);
    assert(needle != NULL);

#ifdef GLOB_BRACE
    /* this is a GNU extension, see glob(3) */
    gflags |= GLOB_BRACE;
#endif

    n = strdup(needle);
    assert(n != NULL);

    if (root != NULL) {
        len = strlen(root);
    }

    memset(globpath, 0, sizeof(globpath));

    if (root != NULL) {
        gp = stpcpy(gp, root);
    }

    if (!strprefix(pattern, "/")) {
        gp = stpcpy(gp, dirname(n));
    }

    free(n);

    if (!strsuffix(globpath, "/") && !strprefix(pattern, "/")) {
        gp = stpcpy(gp, "/");
    }

    gp = stpcpy(gp, pattern);
    DEBUG_PRINT("globpath=|%s|\n", globpath);
    r = glob(globpath, gflags, NULL, &found);

    if (r == GLOB_NOSPACE || r == GLOB_ABORTED) {
        warn("glob()");
    }

    if (r != 0) {
        return false;
    }

    for (i = 0; i < found.gl_pathc; i++) {
        globsub = found.gl_pathv[i] + len;

        if (!strcmp(globsub, needle)) {
            match = true;
            break;
        }
    }

    globfree(&found);

    return match;
}

/**
 * @brief Given a path and struct rpminspect, determine if the path should be ignored or not.
 *
 * @param ri The struct rpminspect for the program.
 * @param inspection The name of the inspection currently running.
 * @param path The relative path to check (i.e., localpath).
 * @param root The root directory, optional (pass NULL to use '/').
 * @return True if path should be ignored, false otherwise.
 */
bool ignore_path(const struct rpminspect *ri, const char *inspection, const char *path, const char *root)
{
    bool match = false;
    string_entry_t *entry = NULL;
    string_list_map_t *mapentry = NULL;

    assert(ri != NULL);

    if (path == NULL) {
        return true;
    }

    DEBUG_PRINT("ignore_path -> path=|%s|\n", path);

    /* first, handle the global ignores */
    if (ri->ignores != NULL && !TAILQ_EMPTY(ri->ignores)) {
        TAILQ_FOREACH(entry, ri->ignores, items) {
            match = match_path(entry->data, root, path);

            if (match) {
                break;
            }
        }
    }

    if (match) {
        return match;
    }

    /* second, handle the per-inspection ignores */
    if (ri->inspection_ignores != NULL) {
        HASH_FIND_STR(ri->inspection_ignores, inspection, mapentry);

        if (mapentry != NULL && mapentry->value != NULL && !TAILQ_EMPTY(mapentry->value)) {
            TAILQ_FOREACH(entry, mapentry->value, items) {
                match = match_path(entry->data, root, path);

                if (match) {
                    return match;
                }
            }
        }
    }

    return match;
}
