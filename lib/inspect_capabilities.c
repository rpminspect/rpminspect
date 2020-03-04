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

static bool capabilities_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    cap_t aftercap = NULL;
    cap_t beforecap = NULL;
    char *after = NULL;
    char *before = NULL;
    char *msg = NULL;
    const char *pkg = NULL;
    const char *arch = NULL;
    const char *name = NULL;
    caps_filelist_entry_t *flcaps = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* Skip debuginfo and debugsource packages */
    name = headerGetString(file->rpm_header, RPMTAG_NAME);
    if (strsuffix(name, DEBUGINFO_SUFFIX) || strsuffix(name, DEBUGSOURCE_SUFFIX)) {
        return true;
    }

    /* Skip any file in a build-id path */
    if (strstr(file->localpath, BUILD_ID_DIR)) {
        return true;
    }

    /* Get the cap values */
    aftercap = get_cap(file);
    after = cap_to_text(aftercap, NULL);

    if (file->peer_file) {
        beforecap = get_cap(file->peer_file);
        before = cap_to_text(beforecap, NULL);
    }

    /* The architecture is used in reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Report if the caps are different */
    if (after && before && strcmp(after, before)) {
        xasprintf(&msg, _("File capabilities for %s changed from '%s' to '%s' on %s\n"), file->localpath, before, after, arch);
        add_result(ri, RESULT_VERIFY, WAIVABLE_BY_SECURITY, HEADER_CAPABILITIES, msg, NULL, REMEDY_CAPABILITIES);
        free(msg);
        result = false;
    } else if (after && before && !strcmp(after, before)) {
        xasprintf(&msg, _("File capabilities found for %s: '%s' on %s\n"), file->localpath, after, arch);
        add_result(ri, RESULT_INFO, NOT_WAIVABLE, HEADER_CAPABILITIES, msg, NULL, NULL);
        free(msg);
    }

    /* If we have after caps, check it against the whitelist and report */
    pkg = headerGetString(file->rpm_header, RPMTAG_NAME);
    flcaps = get_caps_whitelist_entry(ri, pkg, file->localpath);

    if (!after && !flcaps) {
        return true;
    }

    if (after && flcaps) {
        if (!strcmp(after, flcaps->caps)) {
            xasprintf(&msg, _("File capabilities whitelist entry found for %s: '%s' on %s, matches package\n"), file->localpath, flcaps->caps, arch);
            add_result(ri, RESULT_INFO, NOT_WAIVABLE, HEADER_CAPABILITIES, msg, NULL, NULL);
            free(msg);
        } else {
            xasprintf(&msg, _("File capabilities whitelist mismatch for %s: expected '%s', got '%s'\n"), file->localpath, flcaps->caps, arch);
            add_result(ri, RESULT_BAD, WAIVABLE_BY_SECURITY, HEADER_CAPABILITIES, msg, NULL, REMEDY_CAPABILITIES);
            free(msg);
            result = false;
        }
    } else if (after && !flcaps) {
        xasprintf(&msg, _("File capabilities for %s not found on the capabilities whitelist on %s\n"), file->localpath, arch);
        add_result(ri, RESULT_BAD, WAIVABLE_BY_SECURITY, HEADER_CAPABILITIES, msg, NULL, REMEDY_CAPABILITIES);
        free(msg);
        result = false;
    } else if (!after && flcaps) {
        xasprintf(&msg, _("File capabilities expected for %s but not found on %s: expected '%s'\n"), file->localpath, arch, flcaps->caps);
        add_result(ri, RESULT_BAD, WAIVABLE_BY_SECURITY, HEADER_CAPABILITIES, msg, NULL, REMEDY_CAPABILITIES);
        free(msg);
        result = false;
    }

    return result;
}

/*
 * Main driver for the 'capabilities' inspection.
 */
bool inspect_capabilities(struct rpminspect *ri) {
    bool result;

    assert(ri != NULL);

    /* run the capabilities inspection across all RPM files */
    result = foreach_peer_file(ri, capabilities_driver);

    /* if everything was fine, just say so */
    if (result) {
        add_result(ri, RESULT_OK, NOT_WAIVABLE, HEADER_CAPABILITIES, NULL, NULL, NULL);
    }

    return result;
}
