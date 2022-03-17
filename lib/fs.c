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
#include <err.h>
#include <sys/statvfs.h>

#include "rpminspect.h"

unsigned long get_available_space(const char *path)
{
    unsigned long r = 0;
    struct statvfs svb;

    assert(path != NULL);

    if (statvfs(path, &svb) == -1) {
        warn("statvfs");
        return 0;
    }

    r = svb.f_bsize * svb.f_bavail;

    return r;
}
