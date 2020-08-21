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

#include <assert.h>
#include <err.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>

#ifdef _COMPAT_QUEUE
#include "compat/queue.h"
#else
#include <sys/queue.h>
#endif

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
