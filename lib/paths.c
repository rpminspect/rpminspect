/*
 * Copyright (C) 2020  Red Hat, Inc.
 * Red Hat Author(s):  David Cantrell <dcantrell@redhat.com>
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

#include <assert.h>
#include <sys/queue.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>

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
