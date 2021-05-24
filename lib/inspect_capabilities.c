/*
 * Copyright © 2019 Red Hat, Inc.
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
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/capability.h>

#include "rpminspect.h"

static bool capabilities_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    cap_t aftercap = NULL;
    cap_t beforecap = NULL;
    cap_t expected = NULL;
    char *after = NULL;
    char *before = NULL;
    const char *pkg = NULL;
    const char *arch = NULL;
    const char *name = NULL;
    caps_filelist_entry_t *flcaps = NULL;
    struct result_params params;

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

    if (file->peer_file) {
        beforecap = get_cap(file->peer_file);
    }

    /* The architecture is used in reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_CAPABILITIES;
    params.arch = arch;
    params.file = file->localpath;

    /* Report if the caps are different */
    if (beforecap && aftercap) {
        before = cap_to_text(beforecap, NULL);
        assert(before != NULL);
        after = cap_to_text(aftercap, NULL);
        assert(after != NULL);

        if (cap_compare(beforecap, aftercap)) {
            xasprintf(&params.msg, _("File capabilities for %s changed from '%s' to '%s' on %s\n"), file->localpath, before, after, arch);
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_SECURITY;
            params.remedy = REMEDY_CAPABILITIES;
            params.verb = VERB_CHANGED;
            params.noun = _("${FILE} capabilities");
            add_result(ri, &params);
            free(params.msg);
            result = false;
        } else if (!cap_compare(beforecap, aftercap)) {
            xasprintf(&params.msg, _("File capabilities found for %s: '%s' on %s\n"), file->localpath, after, arch);
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
            add_result(ri, &params);
            free(params.msg);
        }
    }

    /* If we have after caps, check it against the caps list and report */
    pkg = headerGetString(file->rpm_header, RPMTAG_NAME);
    flcaps = get_caps_entry(ri, pkg, file->localpath);

    if (!aftercap && !flcaps) {
        return true;
    }

    if (flcaps->caps) {
        expected = cap_from_text(flcaps->caps);
    }

    if (aftercap && expected) {
        if (!cap_compare(aftercap, expected)) {
            xasprintf(&params.msg, _("File capabilities list entry found for %s: '%s' on %s, matches package\n"), file->localpath, flcaps->caps, arch);
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
            add_result(ri, &params);
            free(params.msg);
        } else {
            xasprintf(&params.msg, _("File capabilities list mismatch for %s: expected '%s', got '%s'\n"), file->localpath, flcaps->caps, arch);
            params.severity = RESULT_BAD;
            params.waiverauth = WAIVABLE_BY_SECURITY;
            params.remedy = REMEDY_CAPABILITIES;
            params.verb = VERB_FAILED;
            params.noun = _("${FILE} capabilities list");
            add_result(ri, &params);
            free(params.msg);
            result = false;
        }
    } else if (aftercap && cap_compare(aftercap, expected)) {
        xasprintf(&params.msg, _("File capabilities for %s not found on the capabilities list on %s\n"), file->localpath, arch);
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_SECURITY;
        params.remedy = REMEDY_CAPABILITIES;
        params.verb = VERB_REMOVED;
        params.noun = _("${FILE} capabilities list");
        add_result(ri, &params);
        free(params.msg);
        result = false;
    } else if (!aftercap && expected) {
        xasprintf(&params.msg, _("File capabilities expected for %s but not found on %s: expected '%s'\n"), file->localpath, arch, flcaps->caps);
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_SECURITY;
        params.remedy = REMEDY_CAPABILITIES;
        params.verb = VERB_FAILED;
        params.noun = _("${FILE} capabilities list");
        add_result(ri, &params);
        free(params.msg);
        result = false;
    }

    return result;
}

/*
 * Main driver for the 'capabilities' inspection.
 */
bool inspect_capabilities(struct rpminspect *ri) {
    bool result;
    struct result_params params;

    assert(ri != NULL);

    /* run the capabilities inspection across all RPM files */
    result = foreach_peer_file(ri, NAME_CAPABILITIES, capabilities_driver, true);

    /* if everything was fine, just say so */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_CAPABILITIES;
        add_result(ri, &params);
    }

    return result;
}
