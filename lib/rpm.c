/*
 * Copyright © 2019 Red Hat, Inc.
 * Author(s): David Shea <dshea@redhat.com>
 *            David Cantrell <dcantrell@redhat.com>
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
#include <err.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <rpm/header.h>

#include "rpminspect.h"

/* Initialize librpm if needed */
int init_librpm(struct rpminspect *ri)
{
    int result;

    assert(ri != NULL);

    if (ri->librpm_initialized) {
        return RPMRC_OK;
    }

    result = rpmReadConfigFiles(NULL, NULL);

    if (result == RPMRC_OK) {
        ri->librpm_initialized = true;
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
    header_cache_t *hentry = NULL;

    assert(ri != NULL);
    assert(pkg != NULL);

    /* The cache stores the basename of the pkg */
    bpkg = basename(pkg);

    /* First see if we can return the cached header */
    if (ri->header_cache != NULL) {
        HASH_FIND_STR(ri->header_cache, bpkg, hentry);

        if (hentry != NULL) {
            return hentry->hdr;
        }
    }

    /* No?  Then read the header in, cache it, and return it. */
    fd = Fopen(pkg, "r.ufdio");

    if (fd == NULL || Ferror(fd)) {
        warnx(_("Fopen failed for %s: %s"), pkg, Fstrerror(fd));

        if (fd) {
            Fclose(fd);
        }

        return NULL;
    }

    hentry = calloc(1, sizeof(*hentry));
    assert(hentry != NULL);
    hentry->pkg = strdup(bpkg);
    assert(hentry->pkg != NULL);

    ts = rpmtsCreate();
    rpmtsSetVSFlags(ts, _RPMVSF_NODIGESTS | _RPMVSF_NOSIGNATURES);
    result = rpmReadPackageFile(ts, fd, pkg, &hentry->hdr);
    rpmtsFree(ts);
    Fclose(fd);

    if (result != RPMRC_OK) {
        free(hentry->pkg);
        free(hentry);
        return NULL;
    }

    HASH_ADD_KEYPTR(hh, ri->header_cache, hentry->pkg, strlen(hentry->pkg), hentry);
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

/**
 * @brief Given an RPM header and a tag constant, retrieve the tag
 * value and add each array element as a member of a newly allocated
 * string_list_t.  The caller is responsible for freeing the retruned
 * string_list_t.  The tag constant must be a string array in the RPM
 * header.  If the tag cannot be found or is empty, this function
 * returns NULL.
 *
 * @param hdr RPM header
 * @param tag RPM header tag constant (e.g., RPMTAG_SOURCE)
 * @return Newly allocated string_list_t containg tag values.
 */
string_list_t *get_rpm_header_string_array(Header hdr, rpmTagVal tag)
{
    string_list_t *list = NULL;
    rpmtd td = NULL;
    rpmFlags flags = HEADERGET_MINMEM | HEADERGET_EXT | HEADERGET_ARGV;
    string_entry_t *entry = NULL;
    const char *val = NULL;

    if (hdr == NULL) {
        return NULL;
    }

    td = rpmtdNew();

    if (!headerGet(hdr, tag, td, flags)) {
        rpmtdFree(td);
        return NULL;
    }

    /* walk the tag and cram everything in to a list */
    list = calloc(1, sizeof(*list));
    assert(list != NULL);
    TAILQ_INIT(list);

    while ((val = rpmtdNextString(td)) != NULL) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->data = strdup(val);
        TAILQ_INSERT_TAIL(list, entry, items);
    }

    rpmtdFree(td);
    return list;
}

/*
 * Given an RPM header tag, get that header tag array and return the
 * string that matches the index value for this file.  That's complex,
 * but some tags are arrays of strings (or ints) and what we need to
 * do is first get the array, then knowing the index entry for the file
 * we have, pull that array index out and return it.  NULL return means
 * an empty value or the tag was not present in the header.
 *
 * Limitations:
 * "tag" must refer to an s[] tag (see rpmtag.h from librpm)
 * "file" must have a usable array index value (idx)
 *
 * Returned value must be free'd by caller.
 */
char *get_rpm_header_value(const rpmfile_entry_t *file, rpmTag tag)
{
    rpmtd td = NULL;
    rpmFlags flags = HEADERGET_MINMEM | HEADERGET_EXT | HEADERGET_ARGV;
    const char *val = NULL;
    char *ret = NULL;

    assert(file != NULL);
    assert(file->idx >= 0);

    /* new header transaction */
    td = rpmtdNew();

    /* find the header tag we want to extract values from */
    if (!headerGet(file->rpm_header, tag, td, flags)) {
        rpmtdFree(td);
        return ret;
    }

    /* set the array index */
    if (rpmtdSetIndex(td, file->idx) == -1) {
        warn(_("*** file index %d is out of bounds for %s"), file->idx, file->fullpath);
        rpmtdFree(td);
        return ret;
    }

    /* get the tag we are looking for and copy the value */
    val = rpmtdGetString(td);
    assert(val != NULL);
    ret = strdup(val);
    rpmtdFree(td);

    return ret;
}
