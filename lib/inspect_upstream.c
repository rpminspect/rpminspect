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
#include <assert.h>
#include <libgen.h>
#include <sys/queue.h>

#include "rpminspect.h"

static string_list_t *source = NULL;
static const char *ver_before = NULL;
static const char *ver_after = NULL;
static const char *epoch_before = NULL;
static const char *epoch_after = NULL;
static bool same = false;

/*
 * Get the SOURCE tag from the RPM header and read in all of
 * the values from that tag and put them in the 'source' list.
 */
static void init_source(const rpmfile_entry_t *file)
{
    rpmtd td = NULL;
    rpmFlags flags = HEADERGET_MINMEM | HEADERGET_EXT | HEADERGET_ARGV;
    string_entry_t *entry = NULL;
    const char *val = NULL;

    assert(file != NULL);

    /* get the SOURCE tag */
    td = rpmtdNew();

    if (!headerGet(file->rpm_header, RPMTAG_SOURCE, td, flags)) {
        fprintf(stderr, "*** unable to find RPMTAG_SOURCE for %s\n", file->fullpath);
        abort();
    }

    /* walk the SOURCE tag and cram everything in to a list */
    source = calloc(1, sizeof(*source));
    assert(source != NULL);
    TAILQ_INIT(source);

    while ((val = rpmtdNextString(td)) != NULL) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->data = strdup(val);
        TAILQ_INSERT_TAIL(source, entry, items);
    }

    rpmtdFreeData(td);
    rpmtdFree(td);

    return;
}

/* Returns true if this file is a Source file */
static bool is_source(const rpmfile_entry_t *file)
{
    bool ret = false;
    char *shortname = NULL;
    string_entry_t *entry = NULL;

    assert(file != NULL);

    /* Initialize the source list once for the run */
    if (source == NULL) {
        init_source(file);
    }

    /* The RPM header stores basenames */
    shortname = basename(file->fullpath);

    /* See if this file is a Source file */
    TAILQ_FOREACH(entry, source, items) {
        if (!strcmp(shortname, entry->data)) {
            ret = true;
            break;
        }
    }

    return ret;
}

/* Main driver for the 'upstream' inspection. */
static bool upstream_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    char *before_sum = NULL;
    char *after_sum = NULL;
    char *shortname = NULL;
    char *msg = NULL;

    /* If we are not looking at a Source file, bail. */
    if (!is_source(file)) {
        return true;
    }

    /* Gather the version and epoch information */
    ver_before = headerGetString(file->peer_file->rpm_header, RPMTAG_VERSION);
    ver_after = headerGetString(file->rpm_header, RPMTAG_VERSION);
    epoch_before = headerGetString(file->peer_file->rpm_header, RPMTAG_EPOCH);
    epoch_after = headerGetString(file->rpm_header, RPMTAG_EPOCH);

    /*
     * Versions and/or epoch differ?  Don't care is sources change.
     * NOTE: librpm will happily give us NULL if there is no Epoch
     */
    if (strcmp(ver_before, ver_after)) {
        return true;
    }

    if ((epoch_before == NULL && epoch_after) ||
        ((epoch_before && epoch_after) && strcmp(epoch_before, epoch_after))) {
        return true;
    }

    /* Compare digests of source archive */
    same = true;
    shortname = basename(file->fullpath);

    if (file->peer_file == NULL) {
        xasprintf(&msg, "New upstream source file `%s` appeared, but the build version (%s) and epoch (%s) remained the same", shortname, ver_after, epoch_after);
        add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_UPSTREAM, msg, NULL, REMEDY_UPSTREAM);
        result = false;
    } else {
        before_sum = checksum(file->peer_file->fullpath, &file->peer_file->st.st_mode, SHA256SUM);
        after_sum = checksum(file->fullpath, &file->st.st_mode, SHA256SUM);

        if (strcmp(before_sum, after_sum)) {
            xasprintf(&msg, "Upstream source file `%s` changed content, but the build version (%s) and epoch (%s) remained the same", shortname, ver_after, epoch_after);
            add_result(ri, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_UPSTREAM, msg, NULL, REMEDY_UPSTREAM);
            result = false;
        }
    }

    free(msg);
    free(before_sum);
    free(after_sum);
    return result;
}

/*
 * Main driver for the 'upstream' inspection.
 */
bool inspect_upstream(struct rpminspect *ri)
{
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    char *msg = NULL;

    assert(ri != NULL);

    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Only look at the files in SRPMs */
        if (!headerIsSource(peer->after_hdr)) {
            continue;
        }

        /* On the off chance the SRPM is empty, just ignore */
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        /* Iterate over the SRPM files */
        TAILQ_FOREACH(file, peer->after_files, items) {
            if (!upstream_driver(ri, file)) {
                result = false;
            }
        }

        /* Report any removed source files from the SRPM */
        if (same) {
            TAILQ_FOREACH(file, peer->before_files, items) {
                if (file->peer_file == NULL) {
                    xasprintf(&msg, "Source RPM member `%s` removed, but the build version (%s) and epoch (%s) remained the same", basename(file->fullpath), ver_after, epoch_after);
                    add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_UPSTREAM, msg, NULL, REMEDY_UPSTREAM);
                    result = false;
                }
            }
        }
    }

    /* Sound the everything-is-ok alarm if everything is, in fact, ok */
    if (result) {
        add_result(ri, RESULT_OK, NOT_WAIVABLE, HEADER_UPSTREAM, NULL, NULL, NULL);
    }

    /* Our static list of SourceN: spec file members, dump it */
    list_free(source, free);

    return result;
}
