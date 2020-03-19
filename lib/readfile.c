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

#include <assert.h>
#include <errno.h>

#include "rpminspect.h"

/*
 * Open and read the contents of a file line by line in to a string_list_t.
 * Each line is a separate entry.  Caller must call listfree() on the list
 * returned.
 */
string_list_t *read_file(const char *path)
{
    FILE *fp = NULL;
    string_list_t *data = NULL;
    string_entry_t *entry = NULL;
    char *buf = NULL;
    size_t buflen = 0;

    assert(path != NULL);
    fp = fopen(path, "r");

    if (fp == NULL) {
        fprintf(stderr, "*** %s (%d): %s\n", __func__, __LINE__, strerror(errno));
        return NULL;
    }

    data = calloc(1, sizeof(*data));
    assert(data != NULL);
    TAILQ_INIT(data);

    while (getline(&buf, &buflen, fp) != -1) {
        entry = calloc(1, sizeof(*entry));
        entry->data = buf;
        TAILQ_INSERT_TAIL(data, entry, items);
        buf = NULL;
    }

    if (fclose(fp) == -1) {
        fprintf(stderr, "*** %s (%d): %s\n", __func__, __LINE__, strerror(errno));
        return NULL;
    }

    return data;
}
