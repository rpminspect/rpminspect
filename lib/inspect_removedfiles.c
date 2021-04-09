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
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static void add_removedfiles_result(struct rpminspect *ri, struct result_params *params)
{
    assert(ri != NULL);
    assert(params != NULL);

    if (params->waiverauth == WAIVABLE_BY_SECURITY) {
        params->msg = strappend(params->msg, _("; Removing security policy related files requires inspection by the Security Response Team."), NULL);
    }

    add_result(ri, params);
    return;
}

/*
 * Performs all of the tests associated with the removedfiles inspection.
 * NOTE:  This function is called while looping over before_files.
 */
static bool removedfiles_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    char *type = NULL;
    const char *arch = NULL;
    bool rebase = false;
    char *soname = NULL;
    string_entry_t *entry = NULL;
    struct result_params params;

    /* Any entry with a peer has not been removed. */
    if (file->peer_file) {
        return true;
    }

    /* Skip moved files */
    if (file->moved_path && file->peer_file->moved_path) {
        return true;
    }

    /* Only perform checks on regular files */
    if (!S_ISREG(file->st.st_mode)) {
        return true;
    }

    /*
     * Ignore certain file removals:
     * - Anything in a .build-id/ subdirectory
     * - Any Python egg file ending with .egg-info
     */
    if (strstr(file->localpath, BUILD_ID_DIR) ||
        strsuffix(file->localpath, EGGINFO_FILENAME_EXTENSION)) {
        return true;
    }

    /* Collect the RPM architecture and file MIME type */
    type = get_mime_type(file);
    arch = get_rpm_header_arch(file->rpm_header);
    rebase = is_rebase(ri);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = HEADER_REMOVEDFILES;
    params.arch = arch;
    params.file = file->localpath;

    if (rebase) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
    } else {
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.remedy = REMEDY_REMOVEDFILES;
    }

    /* Set the waiver type if this is a file of security concern */
    if (ri->security_path_prefix) {
        TAILQ_FOREACH(entry, ri->security_path_prefix, items) {
            while (*entry->data != '/') {
                entry->data++;
            }

            if (strprefix(entry->data, file->localpath)) {
                params.severity = RESULT_BAD;
                params.waiverauth = WAIVABLE_BY_SECURITY;
                break;
            }
        }
    }

    /*
     * File has been removed, report results.
     */
    if (is_elf(file->fullpath) && !strcmp(type, "application/x-pie-executable")) {
        soname = get_elf_soname(file->fullpath);
        params.severity = RESULT_BAD;

        if (soname) {
            xasprintf(&params.msg, _("ABI break: Library %s with SONAME '%s' removed from %s"), file->localpath, soname, arch);
            free(soname);
        } else {
            xasprintf(&params.msg, _("ABI break: Library %s removed from %s"), file->localpath, arch);
        }

        add_removedfiles_result(ri, &params);
        result = !(params.severity >= RESULT_VERIFY);
        free(params.msg);
    } else {
        xasprintf(&params.msg, _("%s removed from %s"), file->localpath, arch);
        add_removedfiles_result(ri, &params);
        result = !(params.severity >= RESULT_VERIFY);
        free(params.msg);
    }

    return result;
}

bool inspect_removedfiles(struct rpminspect *ri)
{
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    const char *name = NULL;
    struct result_params params;

    assert(ri != NULL);

    /*
     * This is like our after_files loop helper in inspect.c, but
     * run the loop over the before_files.  This is because we want
     * to check for removed files which is easily detected by a
     * missing peer_file on the before_files list.
     */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Skip source RPMs */
        if (headerIsSource(peer->before_hdr)) {
            continue;
        }

        /* Skip debuginfo and debugsource packages */
        name = headerGetString(peer->before_hdr, RPMTAG_NAME);

        if (strsuffix(name, DEBUGINFO_SUFFIX) || strsuffix(name, DEBUGSOURCE_SUFFIX)) {
            continue;
        }

        /* If there are no peer files */
        if (peer->before_files == NULL) {
            continue;
        }

        /* Iterate over all files in the before package */
        TAILQ_FOREACH(file, peer->before_files, items) {
            if (!removedfiles_driver(ri, file)) {
                result = false;
            }
        }
    }

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_REMOVEDFILES;
        add_result(ri, &params);
    }

    return result;
}
