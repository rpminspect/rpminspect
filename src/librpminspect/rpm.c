/*
 * Copyright (C) 2019  Red Hat, Inc.
 * Red Hat Author(s):  David Shea <dshea@redhat.com>
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

#include "config.h"

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
