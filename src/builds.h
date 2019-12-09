/*
 * Copyright (C) 2019  Red Hat, Inc.
 * Author(s):  David Cantrell <dcantrell@redhat.com>
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

#ifndef _RPMINSPECT_BUILDS_H
#define _RPMINSPECT_BUILDS_H

#include "rpminspect.h"

/* builds.c */
int gather_builds(struct rpminspect *, bool);

/* Types of workdirs */
typedef enum _workdir_t {
    NULL_WORKDIR = 0,          /* unused                    */
    LOCAL_WORKDIR = 1,         /* locally cached koji build */
    TASK_WORKDIR = 2,          /* like for scratch builds   */
    BUILD_WORKDIR = 3          /* remote koji build spec    */
} workdir_t;

#endif
