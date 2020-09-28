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

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "rpminspect.h"

/*
 * Compares two files.  The return value of this function matches what
 * memcmp() returns (mostly).  The function will return 1 if the sizes
 * read of each file are different and it will skip the comparison
 * entirely.
 */
int filecmp(const char *x, const char *y)
{
    int r = 0;
    void *xbuf = NULL;
    void *ybuf = NULL;
    off_t xlen = 0;
    off_t ylen = 0;

    assert(x != NULL);
    assert(y != NULL);

    /* read in the files */
    xbuf = read_file_bytes(x, &xlen);
    assert(xbuf != NULL);

    ybuf = read_file_bytes(y, &ylen);
    assert(ybuf != NULL);

    /* they are different if the sizes read are different */
    if (xlen != ylen) {
        free(xbuf);
        free(ybuf);
        return 1;
    }

    r = memcmp(xbuf, ybuf, xlen);
    free(xbuf);
    free(ybuf);

    return r;
}
