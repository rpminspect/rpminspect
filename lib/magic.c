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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <magic.h>

#include "rpminspect.h"

/*
 * Return the MIME type of the specified file.  The type is cached in the
 * rpmfile_entry_t.  If that is not NULL, this function returns that value.
 * Otherwise it gets the MIME type, caches it, and returns the value.
 * The caller should not free the pointer returned.
 */
char *get_mime_type(rpmfile_entry_t *file) {
    char *ret = NULL;
    char *pos = NULL;
    const char *tmp = NULL;
    magic_t cookie;

    assert(file != NULL);

    /* MIME type is cached, return it */
    if (file->type != NULL) {
        return file->type;
    }

    /* Get and cache MIME type */
    assert(file->fullpath != NULL);
    cookie = magic_open(MAGIC_MIME | MAGIC_CHECK);

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

    if ((tmp = magic_file(cookie, file->fullpath)) != NULL) {
        file->type = strdup(tmp);

        /*
         * Trim any trailing metadata after the MIME type, such
         * as 'charset=binary' and stuff like that.
         */
        if ((pos = index(file->type, ';')) != NULL) {
            *pos = '\0';
            file->type = realloc(file->type, strlen(file->type) + 1);
        }
    }

    magic_close(cookie);
    return file->type;
}

/* Return true if the named file is a text file according to libmagic */
bool is_text_file(rpmfile_entry_t *file)
{
    bool ret = false;
    char *type = NULL;

    assert(file != NULL);
    type = get_mime_type(file);

    if (strprefix(type, "text/")) {
        ret = true;
    }

    return ret;
}
