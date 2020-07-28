/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
 * Red Hat Author(s):  David Shea <dshea@redhat.com>
 *                     David Cantrell <dcantrell@redhat.com>
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

#include <stdbool.h>
#include <assert.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <rpm/header.h>

#include "rpminspect.h"

/* Initialize librpm if needed */
int init_librpm(void)
{
    int result;
    static bool initialized = false;

    if (initialized) {
        return RPMRC_OK;
    }

    result = rpmReadConfigFiles(NULL, NULL);
    if (result == RPMRC_OK) {
        initialized = true;
    }

    return result;
}

/* Return an RPM header struct for the given package filename. */
Header get_rpm_header(struct rpminspect *ri, const char *pkg)
{
    rpmts ts;
    FD_t fd;
    rpmRC result;
    char *bpkg = NULL;
    header_cache_entry_t *hentry = NULL;

    assert(ri != NULL);
    assert(pkg != NULL);

    /* The cache stores the basename of the pkg */
    bpkg = basename(pkg);

    /* First see if we can return the cached header */
    if (ri->header_cache != NULL) {
        TAILQ_FOREACH(hentry, ri->header_cache, items) {
            if (!strcmp(hentry->pkg, bpkg)) {
                return hentry->hdr;
            }
        }
    }

    /* No?  Then read the header in, cache it, and return it. */
    fd = Fopen(pkg, "r.ufdio");

    if (fd == NULL || Ferror(fd)) {
        fprintf(stderr, _("*** Fopen() failed for %s: %s\n"), pkg, Fstrerror(fd));
        fflush(stderr);

        if (fd) {
            Fclose(fd);
        }

        return NULL;
    }

    hentry = calloc(1, sizeof(*hentry));
    hentry->pkg = strdup(bpkg);

    ts = rpmtsCreate();
    rpmtsSetVSFlags(ts, _RPMVSF_NODIGESTS | _RPMVSF_NOSIGNATURES);
    result = rpmReadPackageFile(ts, fd, pkg, &hentry->hdr);
    rpmtsFree(ts);
    Fclose(fd);

    if (result != RPMRC_OK) {
        return NULL;
    }

    if (ri->header_cache == NULL) {
        /* Initialize the header cache if necessary */
        ri->header_cache = calloc(1, sizeof(*ri->header_cache));
        assert(ri->header_cache != NULL);
        TAILQ_INIT(ri->header_cache);
    }

    TAILQ_INSERT_TAIL(ri->header_cache, hentry, items);
    return hentry->hdr;
}

/*
 * Get and return the named RPM header tag as a string.
 */
char *get_rpmtag_str(Header hdr, rpmTagVal tag)
{
    char *val = NULL;
    rpmtd td = NULL;
    rpm_count_t td_size;

    /* no header means no tag value */
    if (hdr == NULL) {
        return NULL;
    }

    td = rpmtdNew();
    assert(td != NULL);

    /* NOTE: this function returns 1 for success, not RPMRC_OK */
    if (headerGet(hdr, tag, td, HEADERGET_MINMEM | HEADERGET_EXT) != 1) {
        goto val_cleanup;
    }

    td_size = rpmtdCount(td);

    if (td_size != 1) {
        goto val_cleanup;
    }

    val = strdup(rpmtdGetString(td));

val_cleanup:
    rpmtdFree(td);
    return val;
}

/*
 * Get the RPMTAG_NEVR extension tag.
 * NOTE: Caller must free this result.
 */
char *get_nevr(Header hdr)
{
    return get_rpmtag_str(hdr, RPMTAG_NEVR);
}

/*
 * Get the RPMTAG_NEVRA extension tag.
 * NOTE: Caller must free this result.
 */
char *get_nevra(Header hdr)
{
    return get_rpmtag_str(hdr, RPMTAG_NEVRA);
}

/*
 * Returns the RPMTAG_ARCH or "src" if it's a source RPM.
 * NOTE: Do not free() what this function returns.
 */
const char *get_rpm_header_arch(Header h)
{
    assert(h != NULL);

    if (headerIsSource(h)) {
        return SRPM_ARCH_NAME;
    } else {
        return headerGetString(h, RPMTAG_ARCH);
    }
}
