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
#include <errno.h>

#include "rpminspect.h"

static bool _disttag_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    bool result = true;
    FILE *fp = NULL;
    size_t len = 0;
    char *buf = NULL;
    char *msg = NULL;
    char *specfile = NULL;

    /* Skip binary packages */
    if (!headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Spec files are all named in a standard way */
    xasprintf(&specfile, "%s.spec", headerGetAsString(file->rpm_header, RPMTAG_NAME));

    /* We only want to look at the spec files */
    if (strcmp(file->localpath, specfile)) {
        free(specfile);
        return true;
    }

    /* We're done with this */
    free(specfile);

    /* Check for the %{?dist} macro in the Release value */
    fp = fopen(file->fullpath, "r");

    if (fp == NULL) {
        fprintf(stderr, "error opening %s for reading: %s\n", file->fullpath, strerror(errno));
        fflush(stderr);
        return false;
    }

    while (getline(&buf, &len, fp) != -1) {
        /* trim line endings */
        buf[strcspn(buf, "\r\n")] = 0;

        /* we made it to the changelog, nothing left of value */
        if (strprefix(buf, "%changelog")) {
            break;
        }

        /* found the line to check */
        if (strprefix(buf, "Release:")) {
            break;
        }

        free(buf);
        buf = NULL;
    }

    if (fclose(fp) == -1) {
        fprintf(stderr, "error closing %s: %s\n", file->fullpath, strerror(errno));
        fflush(stderr);
        result = false;
    }

    /* Check the line if we found it */
    if (buf == NULL) {
        msg = strdup("The %s file is missing the Release: tag.");
        add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_DISTTAG, msg, buf, REMEDY_DISTTAG);
        free(msg);
    } else if (strstr(buf, "dist")) {
        if (strstr(buf, "%{?dist}") == NULL) {
            msg = strdup("The dist tag should be of the form '%%{?dist}' in the Release tag.");
            add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DISTTAG, msg, buf, REMEDY_DISTTAG);
            free(msg);
        }
    } else {
        msg = strdup("The Release: tag does not seem to contain a '%%{?dist}' tag.");
        add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DISTTAG, msg, buf, REMEDY_DISTTAG);
        free(msg);
    }

    free(buf);
    return result;
}

/*
 * Main driver for the 'disttag' inspection.
 */
bool inspect_disttag(struct rpminspect *ri) {
    bool result = false;

    assert(ri != NULL);
    result = foreach_peer_file(ri, _disttag_driver);

    if (result) {
        add_result(&ri->results, RESULT_OK, WAIVABLE_BY_ANYONE, HEADER_DISTTAG, NULL, NULL, NULL);
    }

    return result;
}
