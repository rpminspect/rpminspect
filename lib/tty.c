/*
 * Copyright 2019 Red Hat, Inc.
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

#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include "rpminspect.h"

/*
 * Return the terminal width.  Used by output routines sending text to
 * stdout.  This function returns DEFAULT_TTY_WIDTH if it cannot
 * figure out the width.
 */
size_t tty_width(void)
{
    struct winsize w;

    /* get the terminal size */
    if (ioctl(0, TIOCGWINSZ, &w) == -1) {
        /*
         * We couldn't determine the real size,
         * so let's go with the default width.
         */
        return DEFAULT_TTY_WIDTH;
    }

    return w.ws_col;
}
