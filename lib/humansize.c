/*
 * Copyright 2022 Red Hat, Inc.
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

#include <assert.h>
#include <math.h>

#include "rpminspect.h"

char *human_size(const unsigned long bytes)
{
    char *r = NULL;
    double s = 0;
    char *units = "BKMGTPEZY";
    char *begin = units;

    s = (double) bytes;
    units = begin;

    while ((s > 1024) && (*(units + 1) != '\0')) {
        s = round(s / 1024);
        units++;
    }

    xasprintf(&r, "%lu %c%s", lround(s), *units, (*units == 'B') ? "" : "iB");
    return r;
}
