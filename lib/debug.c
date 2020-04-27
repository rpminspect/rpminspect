/*
 * Copyright (C) 2020  Red Hat, Inc.
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

/**
 * @file debug.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2020
 * @brief Debugging utility functions.
 * @copyright GPL-3.0-or-later
 */

#include "rpminspect.h"

/**
 * @brief Set the global debugging mode.
 *
 * Pass true to enable debugging messages, false to disable.  Usually
 * used in a frontend program after reading in configuration files but
 * before collecting builds and running inspections.
 *
 * @param debug True to enable debugging messages, false to disable.
 */
void set_debug_mode(bool debug)
{
    debug_mode = debug;
    return;
}
