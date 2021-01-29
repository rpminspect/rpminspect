/*
 * Copyright (C) 2019-2021  Red Hat, Inc.
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

/**
 * @file flags.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019-2020
 * @brief Command line handling for inspection flags.
 * @copyright LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#include "rpminspect.h"

/**
 * @brief Process command line options to enable or disable
 *        inspections.
 *
 * Used in the -T and -E option processing to handle each test
 * flag as well as processing the [inspections] section from the
 * configuration file.
 *
 * @param inspection The name of the inspection from the command line.
 *                   e.g., "-T license,manpage" would make two calls
 *                   to this function with inspection being "license"
 *                   and then "manpage".
 * @param exclude True if the flags should be treated as exclusions,
 *                false otherwise.
 * @param selected Pointer to a test bitmap from the caller.
 * @return True if the inspection name is valid, false otherwise.
 */
bool process_inspection_flag(const char *inspection, const bool exclude, uint64_t *selected)
{
    int i = 0;
    bool found = false;

    assert(inspection != NULL);
    assert(selected != NULL);

    if (!strcasecmp(inspection, "ALL")) {
        /* ALL tests specified */
        if (exclude) {
            *selected = 0;
            return true;
        } else {
            *selected = ~0;
            return true;
        }
    }

    for (i = 0; inspections[i].name != NULL; i++) {
        if (!strcasecmp(inspection, inspections[i].name)) {
            /* user specified a valid inspection */
            if (exclude) {
                *selected &= ~(inspections[i].flag);
                found = true;
                break;
            } else {
                *selected |= inspections[i].flag;
                found = true;
                break;
            }
        }
    }

    return found;
}
