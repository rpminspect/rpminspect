/*
 * Copyright 2020 Red Hat, Inc.
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

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "rpminspect.h"

static bool result = true;

static bool types_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    const char *arch = NULL;
    string_list_t *bmt = NULL;      /* before mime type */
    string_list_t *amt = NULL;      /* after mime type */
    string_entry_t *bce = NULL;     /* before category entry */
    string_entry_t *ace = NULL;     /* after category entry */
    string_entry_t *bsce = NULL;    /* before subcategory entry */
    string_entry_t *asce = NULL;    /* after subcategory entry*/
    char *bnevra = NULL;
    char *anevra = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Skip debuginfo and debugsource packages */
    if (is_debug_or_build_path(file->localpath)) {
        return true;
    }

    /* Files without a peer have to be ignored */
    if (file->peer_file == NULL) {
        return true;
    }

    /* Only run this check on regular files */
    if (!S_ISREG(file->st.st_mode) && !S_ISREG(file->peer_file->st.st_mode)) {
        return true;
    }

    /* Skip spec files in source RPMs */
    if (headerIsSource(file->rpm_header) && strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
        return true;
    }

    /* We need the architecture for reporting */
    arch = get_rpm_header_arch(file->rpm_header);
    assert(arch != NULL);

    bnevra = get_nevra(file->peer_file->rpm_header);
    assert(bnevra != NULL);

    anevra = get_nevra(file->rpm_header);
    assert(anevra != NULL);

    /* prepare for results reporting */
    init_result_params(&params);
    params.severity = RESULT_VERIFY;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = NAME_TYPES;
    params.remedy = REMEDY_TYPES;
    params.arch = arch;
    params.file = file->localpath;
    params.verb = VERB_CHANGED;
    params.noun = _("${FILE} MIME type on ${ARCH}");

    /* Get the MIME types */
    bmt = strsplit(get_mime_type(file->peer_file), "/");
    amt = strsplit(get_mime_type(file), "/");

    /* A MIME type should be category/subcategory, so these lists should be length 2 */
    if (result && list_len(bmt) != 2) {
        xasprintf(&params.msg, _("Unknown MIME type `%s' on %s in %s"), get_mime_type(file->peer_file), file->peer_file->localpath, bnevra);
        result = false;
        goto report;
    }

    if (result && list_len(amt) != 2) {
        xasprintf(&params.msg, _("Unknown MIME type `%s' on %s in %s"), get_mime_type(file), file->localpath, anevra);
        result = false;
        goto report;
    }

    /* Compare the first and second parts of the MIME type */
    bce = TAILQ_FIRST(bmt);
    assert(bce != NULL);
    bsce = TAILQ_NEXT(bce, items);
    assert(bsce != NULL);

    ace = TAILQ_FIRST(amt);
    assert(ace != NULL);
    asce = TAILQ_NEXT(ace, items);
    assert(asce != NULL);

    /*
     * The idea with this comparison is to not fail if the type
     * changes from something like 'text/plain' to 'text/x-makefile'.
     * Check both parts of the MIME type independently so that 'text'
     * matches but the subcategory doesn't, but that's still ok.
     * Let's these changes through but will stop for larger changes
     * like text/plain to application/x-executable.
     */
    if (strcmp(bce->data, ace->data) && strcmp(bsce->data, asce->data)) {
        xasprintf(&params.msg, _("MIME type for %s in %s was `%s/%s' and became `%s/%s'"), file->localpath, anevra, bce->data, bsce->data, ace->data, asce->data);
        result = false;
        goto report;
    }

    /* Final reporting */
report:
    if (!result) {
        add_result(ri, &params);
        free(params.msg);
    }

    /* clean up */
    list_free(bmt, free);
    list_free(amt, free);
    free(bnevra);
    free(anevra);

    return result;
}

/*
 * Main driver for the 'types' inspection.
 */
bool inspect_types(struct rpminspect *ri)
{
    struct result_params params;

    assert(ri != NULL);

    /* run the types inspection across all RPM files */
    foreach_peer_file(ri, NAME_TYPES, types_driver);

    /* if everything was fine, just say so */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_TYPES;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
