/*
 * Copyright (C) 2020  Red Hat, Inc.
 * Author(s):  David Cantrell <dcantrell@redhat.com>
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

#include <assert.h>
#include <err.h>
#include <limits.h>
#include <rpm/rpmfi.h>

#include "rpminspect.h"

static bool doc_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    uint64_t before_doc;
    uint64_t after_doc;
    char *diff_output = NULL;
    int exitcode;
    const char *name = NULL;
    const char *arch = NULL;
    const char *minor = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* no peer file, cannot compare...will be handled by "addedfiles" */
    if (file->peer_file == NULL) {
        return true;
    }

    /* skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* the package name is used for reporting */
    name = headerGetString(file->rpm_header, RPMTAG_NAME);

    /* skip debuginfo and debugsource packages */
    if (strsuffix(name, DEBUGINFO_SUFFIX) || strsuffix(name, DEBUGSOURCE_SUFFIX)) {
        return true;
    }

    /* the architecture is used in reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* set up result parameters */
    init_result_params(&params);
    params.header = HEADER_DOC;
    params.arch = arch;
    params.file = file->localpath;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.remedy = REMEDY_DOC;
    params.verb = VERB_CHANGED;
    params.noun = _("%doc ${FILE}");

    if (is_rebase(ri)) {
        params.severity = RESULT_INFO;
    } else {
        params.severity = RESULT_VERIFY;
    }

    /* verify %doc values */
    before_doc = file->flags & RPMFILE_CONFIG;
    after_doc = file->flags & RPMFILE_CONFIG;

    if (before_doc == after_doc) {
        /* compare the files */
        diff_output = run_cmd(&exitcode, DIFF_CMD, "-q", file->peer_file->fullpath, file->fullpath, NULL);

        if (exitcode == 1) {
            /* the files differ, see if it's only whitespace changes */
            result = false;
            free(diff_output);
            diff_output = run_cmd(&exitcode, DIFF_CMD, "-u", "-w", "-I^#.*", file->peer_file->fullpath, file->fullpath, NULL);

            if (exitcode == 0) {
                minor = _(" (comments/whitespace only)");
            }
        }

        if (result == false) {
            /* always report content changes on %doc files as INFO */
            params.severity = RESULT_INFO;

            xasprintf(&params.msg, _("%%doc file content change for %s in %s on %s%s\n"), file->localpath, name, arch, minor);
            params.details = diff_output;
            add_result(ri, &params);
            free(params.msg);
        }
    } else if (before_doc != after_doc) {
        xasprintf(&params.msg, _("%%doc file change for %s in %s on %s (%smarked as %%doc -> %smarked as %%doc)\n"), file->localpath, name, arch,
                  (before_doc ? "" : _("not ")), (after_doc ? "" : _("not ")));
        add_result(ri, &params);
        free(params.msg);
        result = false;
    }

    return result;
}

/*
 * Main driver for the 'doc' inspection.
 */
bool inspect_doc(struct rpminspect *ri) {
    bool result = true;
    struct result_params params;

    assert(ri != NULL);

    result = foreach_peer_file(ri, doc_driver, true);

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_DOC;
        add_result(ri, &params);
    }

    return result;
}
