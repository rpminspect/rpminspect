/*
 * Copyright (C) 2019  Red Hat, Inc.
 * Red Hat Author(s):  David Shea <dshea@redhat.com>
 *                     David Cantrell <dcantrell@redhat.com>
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

#include <stdbool.h>
#include <assert.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>

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
int get_rpm_header(const char *pkg, Header *hdr) {
    rpmts ts;
    FD_t fd;
    rpmRC result;

    assert(pkg != NULL);

    fd = Fopen(pkg, "r.ufdio");

    if (fd == NULL || Ferror(fd)) {
        fprintf(stderr, "*** Fopen() failed for %s: %s\n", pkg, Fstrerror(fd));
        fflush(stderr);

        if (fd) {
            Fclose(fd);
        }

        return -1;
    }

    ts = rpmtsCreate();
    rpmtsSetVSFlags(ts, _RPMVSF_NODIGESTS | _RPMVSF_NOSIGNATURES);

    result = rpmReadPackageFile(ts, fd, pkg, hdr);

    rpmtsFree(ts);
    Fclose(fd);

    if (result == RPMRC_OK) {
        return 0;
    }

    return -1;
}

/*
 * Get the RPMTAG_NEVRA extension tag.
 * NOTE: Caller must free this result.
 */
char *get_nevra(Header hdr)
{
    char *nevra = NULL;
    rpmtd td = NULL;
    rpm_count_t td_size;

    td = rpmtdNew();
    assert(td != NULL);

    /* NOTE: this function returns 1 for success, not RPMRC_OK */
    if (headerGet(hdr, RPMTAG_NEVRA, td, HEADERGET_MINMEM | HEADERGET_EXT) != 1) {
        goto nevra_cleanup;
    }

    td_size = rpmtdCount(td);

    if (td_size != 1) {
        goto nevra_cleanup;
    }

    nevra = strdup(rpmtdGetString(td));

nevra_cleanup:
    rpmtdFree(td);
    return nevra;
}

/*
 * Returns the RPMTAG_ARCH or "src" if it's a source RPM.
 * NOTE: Do not free() what this function returns.
 */
const char *get_rpm_header_arch(Header h)
{
    assert(h);

    if (headerIsSource(h)) {
        return SRPM_ARCH_NAME;
    } else {
        return headerGetString(h, RPMTAG_ARCH);
    }
}
