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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "rpminspect.h"

/**
 * Given a byte array of a specified length, convert it to a NUL
 * terminated string and return a pointer to the string to the caller.
 * The caller is responsible for freeing the memory associated with
 * this string.
 *
 * @param array The byte array to convert.
 * @param len Number of elements in the byte array.
 * @return Newly allocated string representation of the byte array.
 */
char *bytes_to_str(unsigned char *array, size_t len)
{
    size_t i;
    char *ret = NULL;
    char *walk = NULL;

    if (array == NULL) {
        return NULL;
    }

    ret = calloc(1, sizeof(char) * (len + 1));
    assert(ret != NULL);
    walk = ret;

    for (i = 0; i < len; i++) {
        snprintf(walk, 2, "%c", array[i]);
        walk++;
    }

    return ret;
}
