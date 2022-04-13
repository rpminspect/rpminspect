/*
 * Copyright 2019 Red Hat, Inc.
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

static char *remedy_addedfiles = NULL;

/*
 * Performs all of the tests associated with the addedfiles inspection.
 */
static bool addedfiles_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *name = NULL;
    char *subpath = NULL;
    char *localpath = NULL;
    const char *arch = NULL;
    bool rebase = false;
    string_entry_t *entry = NULL;
    struct result_params params;

    /* Ignore source RPMs */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Skip moved files */
    if (file->moved_path && file->peer_file && file->peer_file->moved_path) {
        return true;
    }

    /* Skip debuginfo and debugsource packages */
    name = headerGetString(file->rpm_header, RPMTAG_NAME);

    if (strsuffix(name, DEBUGINFO_SUFFIX) || strsuffix(name, DEBUGSOURCE_SUFFIX)) {
        return true;
    }

    /*
     * Ignore certain file additions:
     * - Anything in a .build-id/ subdirectory
     * - Any Python egg file ending with .egg-info
     */
    if (strstr(file->localpath, BUILD_ID_DIR) ||
        strsuffix(file->localpath, EGGINFO_FILENAME_EXTENSION)) {
        return true;
    }

    /*
     * Security checks first (works for single build runs as well.
     */

    /* The architecture is used in reporting messages */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Is this a rebased build or not? */
    rebase = is_rebase(ri);

    /* Set up the result parameters */
    init_result_params(&params);
    params.severity = RESULT_BAD;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = NAME_ADDEDFILES;
    params.arch = arch;
    params.file = file->localpath;
    params.verb = VERB_FAILED;
    params.remedy = remedy_addedfiles;

    /* Check for any forbidden path prefixes */
    if (ri->forbidden_path_prefixes && (ri->tests & INSPECT_ADDEDFILES)) {
        TAILQ_FOREACH(entry, ri->forbidden_path_prefixes, items) {
            subpath = entry->data;
            localpath = file->localpath;

            /* ensure the paths do not start with '/' */
            if (*subpath == '/') {
                while (*subpath == '/') {
                    subpath++;
                }
            }

            if (*localpath == '/') {
                while (*localpath == '/') {
                    localpath++;
                }
            }

            if (strprefix(localpath, subpath)) {
                xasprintf(&params.msg, _("Packages should not contain files or directories starting with `%s` on %s in %s: %s"), entry->data, arch, name, file->localpath);
                params.noun = _("invalid directory ${FILE} on ${ARCH}");
                add_result(ri, &params);
                result = !(params.severity >= RESULT_VERIFY);
                goto done;
            }
        }
    }

    /* Check for any forbidden path suffixes */
    if (ri->forbidden_path_suffixes && (ri->tests & INSPECT_ADDEDFILES)) {
        TAILQ_FOREACH(entry, ri->forbidden_path_suffixes, items) {
            if (strsuffix(file->localpath, entry->data)) {
                xasprintf(&params.msg, _("Packages should not contain files or directories ending with `%s` on %s in %s: %s"), entry->data, arch, name, file->localpath);
                params.noun = _("invalid directory ${FILE} on ${ARCH}");
                add_result(ri, &params);
                result = !(params.severity >= RESULT_VERIFY);
                goto done;
            }
        }
    }

    /* Check for any forbidden directories */
    if (ri->forbidden_directories && S_ISDIR(file->st.st_mode) && (ri->tests & INSPECT_ADDEDFILES)) {
        TAILQ_FOREACH(entry, ri->forbidden_directories, items) {
            if (!strcmp(file->localpath, entry->data)) {
                xasprintf(&params.msg, _("Forbidden directory `%s` found on %s in %s"), entry->data, arch, name);
                params.noun = _("forbidden directory ${FILE} on ${ARCH}");
                add_result(ri, &params);
                result = !(params.severity >= RESULT_VERIFY);
                goto done;
            }
        }
    }

    /* security path file */
    if (ri->security_path_prefix
        && ri->before != NULL
        && S_ISREG(file->st.st_mode)
        && (file->peer_file == NULL || strcmp(file->localpath, file->peer_file->localpath))) {
        TAILQ_FOREACH(entry, ri->security_path_prefix, items) {
            subpath = entry->data;

            while (*subpath != '/') {
                subpath++;
            }

            if (strprefix(file->localpath, subpath)) {
                params.severity = get_secrule_result_severity(ri, file, SECRULE_SECURITYPATH);

                if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                    params.waiverauth = WAIVABLE_BY_SECURITY;
                    xasprintf(&params.msg, _("New security-related file `%s` added on %s in %s requires inspection by the Security Team"), file->localpath, arch, name);
                    params.verb = VERB_ADDED;
                    params.noun = _("new security-related file ${FILE} on ${ARCH}");
                    add_result(ri, &params);
                    result = !(params.severity >= RESULT_VERIFY);
                } else {
                    result = true;
                }

                goto done;
            }
        }
    }

    /* Check for any new setuid or setgid files */
    if (!S_ISDIR(file->st.st_mode) && (file->st.st_mode & (S_ISUID|S_ISGID))) {
        match_fileinfo_mode(ri, file, NAME_ADDEDFILES, remedy_addedfiles);
        goto done;
    }

    /*
     * Report that a new file has been added in a build comparison.
     */
    if ((ri->tests & INSPECT_ADDEDFILES) && (ri->before != NULL && file->peer_file == NULL)) {
        if (rebase) {
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
            params.verb = VERB_OK;
        } else {
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.verb = VERB_ADDED;
        }

        xasprintf(&params.msg, _("`%s` added on %s in %s"), file->localpath, arch, name);
        params.noun = _("new file ${FILE} on ${ARCH}");
        add_result(ri, &params);
        result = !(params.severity >= RESULT_VERIFY);
    }

done:
    free(params.msg);

    return result;
}

bool inspect_addedfiles(struct rpminspect *ri)
{
    bool result;
    struct result_params params;

    xasprintf(&remedy_addedfiles, REMEDY_ADDEDFILES, ri->fileinfo_filename ? ri->fileinfo_filename : _("the fileinfo list"));
    assert(remedy_addedfiles != NULL);
    result = foreach_peer_file(ri, NAME_ADDEDFILES, addedfiles_driver);
    free(remedy_addedfiles);

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_ADDEDFILES;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
