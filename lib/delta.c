/*
 * Copyright The rpminspect Project Authors
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
#include "xdiff.h"
#include "rpminspect.h"

static int fill_mmfile(mmfile_t *mf, const char *file)
{
    int fd = 0;
    struct stat sb;
    char *buf = NULL;
    unsigned long size = 0;
    int retval = 0;

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

    /* extra byte for the \0 */
    size = sb.st_size;
    buf = calloc(1, size + 1);

    if (buf == NULL) {
        warn("malloc");
        return 1;
    }

    mf->ptr = buf;
    mf->ptr[size] = '\0';
    mf->size = size;

    while (size) {
        retval = read(fd, buf, size);

        if (retval < 0) {
            if (errno == EINTR || errno == EAGAIN) {
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

static int delta_out(void *priv, mmbuffer_t *mb, int nbuf)
{
    int i = 0;
    char *prefix = NULL;
    string_list_t *list = (string_list_t *) priv;
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

        if ((mb[i].size > 1) && mb[i].ptr != NULL) {
            if (prefix) {
                xasprintf(&entry->data, "%s%s", prefix, mb[i].ptr);
            } else {
                entry->data = calloc(1, mb[i].size + 1);
                assert(entry->data != NULL);
                entry->data = strncpy(entry->data, mb[i].ptr, mb[i].size);
            }
        } else {
            entry->data = strdup("");
        }

        assert(entry->data != NULL);
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
    string_list_t *list = NULL;
    char *r = NULL;

    if (fill_mmfile(&old, a) < 0 || fill_mmfile(&new, b) < 0) {
        warn("fill_mmfile");
        return NULL;
    }

    memset(&xpp, 0, sizeof(xpp));
    memset(&xecfg, 0, sizeof(xecfg));
    memset(&ecb, 0, sizeof(ecb));

    list = calloc(1, sizeof(*list));
    assert(list != NULL);
    TAILQ_INIT(list);

    xpp.flags = 0;
    xpp.flags |= XDF_IGNORE_WHITESPACE;

    xecfg.ctxlen = 3;
    ecb.priv = list;
    ecb.outf = delta_out;

    if (xdl_diff(&old, &new, &xpp, &xecfg, &ecb) < 0) {
        warn("xdl_diff");
    }

    if (TAILQ_EMPTY(list)) {
        free(old.ptr);
        free(new.ptr);
        free(list);
        return NULL;
    }

    r = list_to_string(list, "\n");
    list_free(list, free);
    free(old.ptr);
    free(new.ptr);

    return r;
}
