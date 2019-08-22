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

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <magic.h>

#include "rpminspect.h"

/*
 * Return the MIME type of the specified file.  The caller
 * is responsible for free'ing the string returned by this
 * function.
 */
char *get_mime_type(const char *filepath) {
    char *ret = NULL;
    char *pos = NULL;
    const char *tmp = NULL;
    magic_t cookie;

    assert(filepath != NULL);
    cookie = magic_open(MAGIC_MIME);

    if (cookie == NULL) {
        fprintf(stderr, "*** Unable to initialize the magic library\n");
        fflush(stderr);
        return ret;
    }

    if (magic_load(cookie, NULL) != 0) {
        fprintf(stderr, "*** Unable to load the magic database: %s\n", magic_error(cookie));
        fflush(stderr);
        magic_close(cookie);
        return ret;
    }

    if ((tmp = magic_file(cookie, filepath)) != NULL) {
        ret = strdup(tmp);

        /*
         * Trim any trailing metadata after the MIME type, such
         * as 'charset=binary' and stuff like that.
         */
        if ((pos = index(ret, ';')) != NULL) {
            *pos = '\0';
            ret = realloc(ret, strlen(ret) + 1);
        }
    }

    magic_close(cookie);
    return ret;
}
