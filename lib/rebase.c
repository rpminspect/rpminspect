/*
 * Copyright (C) 2020  Red Hat, Inc.
 * Author(s):  David Cantrell <dcantrell@redhat.com>
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

/**
 * @file rebase.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2020
 * @brief Debugging utility functions.
 * @copyright LGPL-3.0-or-later
 */

#include <assert.h>
#include <err.h>
#include "rpminspect.h"

/**
 * @brief Determine if the program is inspecting a rebase build.  That
 * is, the package name of the before and after build match but the
 * versions are different.  If there is no before package, then this
 * function returns true.
 *
 * The value is cached in ri.  By default the value is 0, which
 * indicates it is not set.  This function will set it and a value < 0
 * means false and a value > 0 means true.
 *
 * @param ri The struct rpminspect structure for the program.
 * @return True if the packages are a rebase, false otherwise
 */
bool is_rebase(struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;
    const char *bn = NULL;
    const char *an = NULL;
    const char *bv = NULL;
    const char *av = NULL;
    string_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    /* disabled with the -n command line option */
    if (!ri->rebase_detection) {
        ri->rebase_build = -1;
        return false;
    }

    /*  run the rebase detection */
    if (ri->rebase_build == 0) {
        ri->rebase_build = -1;

        TAILQ_FOREACH(peer, ri->peers, items) {
            /* take the first package where we have each peer */
            if (peer->before_hdr && peer->after_hdr) {
                break;
            }
        }

        if (peer) {
            if (peer->before_hdr == NULL) {
                ri->rebase_build = 1;
            } else {
                bn = headerGetString(peer->before_hdr, RPMTAG_NAME);
                an = headerGetString(peer->after_hdr, RPMTAG_NAME);
                bv = headerGetString(peer->before_hdr, RPMTAG_VERSION);
                av = headerGetString(peer->after_hdr, RPMTAG_VERSION);

                if (!strcmp(bn, an) && strcmp(bv, av)) {
                    ri->rebase_build = 1;
                }
            }
        }
    }

    /* if the package name is on the rebaseable list, it's valid */
    if (init_rebaseable(ri)) {
        TAILQ_FOREACH(entry, ri->rebaseable, items) {
            if ((bn && !strcmp(entry->data, bn)) && (an && !strcmp(entry->data, an))) {
                return true;
            }
        }
    }

    /* is it a rebase or not or did an error occur? */
    if (ri->rebase_build == -1) {
        return false;
    } else if (ri->rebase_build == 1) {
        return true;
    } else {
        /* should not reach this */
        err(RI_PROGRAM_ERROR, "is_rebase()");
    }
}
