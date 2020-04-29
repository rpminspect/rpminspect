/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
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
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool disttag_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    bool result = true;
    FILE *fp = NULL;
    size_t len = 0;
    char *buf = NULL;
    char *specfile = NULL;
    struct result_params params;

    /* Skip binary packages */
    if (!headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Spec files are all named in a standard way */
    xasprintf(&specfile, "%s.spec", headerGetString(file->rpm_header, RPMTAG_NAME));

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
        fprintf(stderr, _("error opening %s for reading: %s\n"), file->fullpath, strerror(errno));
        fflush(stderr);
        return true;
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
        fprintf(stderr, _("error closing %s: %s\n"), file->fullpath, strerror(errno));
        fflush(stderr);
    }

    /* Set up the result parameters */
    memset(&params, 0, sizeof(params));
    params.severity = RESULT_BAD;
    params.waiverauth = NOT_WAIVABLE;
    params.header = HEADER_DISTTAG;
    params.remedy = REMEDY_DISTTAG;
    params.details = buf;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;

    /* Check the line if we found it */
    if (buf == NULL) {
        xasprintf(&params.msg, _("The %s file is missing the Release: tag."), file->localpath);
        params.verb = VERB_REMOVED;
        params.noun = _("Release: tag");
        add_result(ri, &params);
        result = false;
    } else if (strstr(buf, "dist") && !strstr(buf, "%{?dist}")) {
        params.msg = strdup(_("The dist tag should be of the form '%%{?dist}' in the Release tag."));
        params.verb = VERB_FAILED;
        params.noun = _("'%%{?dist}' tag");
        add_result(ri, &params);
        result = false;
    } else if (!strstr(buf, "%{?dist}")) {
        params.msg = strdup(_("The Release: tag does not seem to contain a '%%{?dist}' tag."));
        params.verb = VERB_REMOVED;
        params.noun = _("'%%{?dist}' tag");
        add_result(ri, &params);
        result = false;
    }

    free(params.msg);
    free(buf);
    return result;
}

/*
 * Main driver for the 'disttag' inspection.
 */
bool inspect_disttag(struct rpminspect *ri) {
    bool result = true;
    bool src = false;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    struct result_params params;

    assert(ri != NULL);

    /*
     * Only run over the package headers and mark if
     * we never see an SRPM file.
     */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (!headerIsSource(peer->after_hdr)) {
            continue;
        }

        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        /* We have at least one source package */
        src = true;

        TAILQ_FOREACH(file, peer->after_files, items) {
            result = disttag_driver(ri, file);
        }
    }

    /* Set up result parameters */
    memset(&params, 0, sizeof(params));
    params.waiverauth = NOT_WAIVABLE;
    params.header = HEADER_DISTTAG;

    /* If we never saw an SRPM, tell the user. */
    if (result && src) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    } else if (!src) {
        params.severity = RESULT_INFO;
        params.msg = _("Specified package is not a source RPM, skipping.");
        add_result(ri, &params);
    }

    return result;
}
