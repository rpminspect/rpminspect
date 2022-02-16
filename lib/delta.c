/*
 * Copyright © 2022 Red Hat, Inc.
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <xdiff.h>
#include "rpminspect.h"

static string_list_t *list = NULL;

static int fill_mmfile(mmfile_t *mf, const char *file)
{
    int fd;
    struct stat sb;
    char *buf = NULL;
    unsigned long size;
    int retval;

    mf->ptr = NULL;
    mf->size = 0;
    fd = open(file, O_RDONLY);

    if (fd < 0) {
        return 0;
    }

    if (stat(file, &sb)) {
        warn("stat");
        return 1;
    }

    size = sb.st_size;
    buf = (char *) malloc(size);

    if (buf == NULL) {
        warn("malloc");
        return 1;
    }

    mf->ptr = buf;
    mf->ptr[size - 1] = '\0';
    mf->size = size;

    while (size) {
        retval = read(fd, buf, size);

        if (retval < 0) {
            if (errno = EINTR || errno == EAGAIN) {
                continue;
            }

            break;
        }

        if (!retval) {
            break;
        }

        buf += retval;
        size -= retval;
    }

    mf->size -= size;

    if (close(fd) < 0) {
        warn("close");
    }

    return 0;
}

static int delta_out(__attribute__((unused)) void *priv, mmbuffer_t *mb, int nbuf)
{
    int i = 0;
    int r = 0;
    char *prefix = NULL;
    string_entry_t *entry = NULL;

    for (i = 0; i < nbuf; i++) {
        /* single byte entries are the +/-/' ' prefix */
        if (mb[i].size == 1 && (mb[i].ptr[0] == ' ' || mb[i].ptr[0] == '+' || mb[i].ptr[0] == '-')) {
            switch (mb[i].ptr[0]) {
                case ' ':
                    prefix = " ";
                    break;
                case '+':
                    prefix = "+";
                    break;
                case '-':
                    prefix = "-";
                    break;
            }

            continue;
        }

        /* capture the line */
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);

        if (prefix) {
            entry->data = calloc(1, mb[i].size + strlen(prefix) + 1);
        } else {
            entry->data = calloc(1, mb[i].size + 1);
        }

        assert(entry->data != NULL);

        if (mb[i].size > 1) {
            if (prefix) {
                r = snprintf(entry->data, mb[i].size + 1, "%s%s", prefix, mb[i].ptr);
            } else {
                r = snprintf(entry->data, mb[i].size + 1, "%s", mb[i].ptr);
            }
        } else {
            free(entry->data);
            entry->data = strdup("");
            assert(entry->data != NULL);
        }

        if (r < 0) {
            warn("snprintf");
            free(entry->data);
            free(entry);
            return -1;
        }

        entry->data[strcspn(entry->data, "\n")] = '\0';
        entry->data = realloc(entry->data, strlen(entry->data) + 1);
        TAILQ_INSERT_TAIL(list, entry, items);
        prefix = NULL;
    }

    return 0;
}

/*
 * Given two paths to files (a and b), load them and generate a
 * unified diff.  The function returns the formatted delta or NULL if
 * there are no differences.
 */
char *get_file_delta(const char *a, const char *b)
{
    mmfile_t old;
    mmfile_t new;
    xpparam_t xpp;
    xdemitconf_t xecfg;
    xdemitcb_t ecb;
    char *r = NULL;

    if (fill_mmfile(&old, a) < 0 || fill_mmfile(&new, b) < 0) {
        warn("fill_mmfile");
        return NULL;
    }

    memset(&xpp, 0, sizeof(xpp));
    memset(&xecfg, 0, sizeof(xecfg));
    memset(&ecb, 0, sizeof(ecb));

    xpp.flags = 0;
    xpp.flags |= XDF_IGNORE_WHITESPACE;

    xecfg.ctxlen = 3;
    ecb.priv = 0;
    ecb.outf = delta_out;

    list = calloc(1, sizeof(*list));
    assert(list != NULL);
    TAILQ_INIT(list);

    if (xdl_diff(&old, &new, &xpp, &xecfg, &ecb) < 0) {
        warn("xdl_diff");
    }

    if (TAILQ_EMPTY(list)) {
        free(list);
        return NULL;
    }

    r = list_to_string(list, "\n");
    list_free(list, free);

    return r;
}
