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

#include <stdbool.h>
#include <assert.h>
#include <fnmatch.h>
#include <err.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include "rpminspect.h"

static bool politics_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    politics_entry_t *pentry = NULL;
    int type = 0;
    char *digest = NULL;
    bool matched = false;
    bool allowed = false;
    int flags = FNM_NOESCAPE | FNM_PERIOD | FNM_EXTMATCH;
    const char *name = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* special files and directories can be skipped */
    if (S_ISDIR(file->st.st_mode) ||
        S_ISCHR(file->st.st_mode) ||
        S_ISBLK(file->st.st_mode) ||
        S_ISFIFO(file->st.st_mode) ||
        S_ISSOCK(file->st.st_mode)) {
        return true;
    }

    /* first pass handles the wildcard entries and sees if we have a match */
    TAILQ_FOREACH(pentry, ri->politics, items) {
        /* malformatted lines */
        if (pentry->pattern == NULL || pentry->digest == NULL) {
            warnx(_("invalid politics entry with pattern=%s and digest=%s"), pentry->pattern, pentry->digest);
            continue;
        }

        /* if we are not looking at a wildcard line, skip */
        if (strcmp(pentry->digest, "*")) {
            continue;
        }

        /* the last entry in the file will take effect here */
        if (!fnmatch(pentry->pattern, file->localpath, flags)) {
            matched = true;
            allowed = pentry->allowed;
        }
    }

    /* look for entries */
    TAILQ_FOREACH(pentry, ri->politics, items) {
        /* malformatted lines */
        if (pentry->pattern == NULL || pentry->digest == NULL) {
            warnx(_("invalid politics entry with pattern=%s and digest=%s"), pentry->pattern, pentry->digest);
            continue;
        }

        /* if we are looking at a wildcard line, skip */
        if (!strcmp(pentry->digest, "*")) {
            continue;
        }

        /* the last entry in the file will take effect here */
        if (!fnmatch(pentry->pattern, file->localpath, flags)) {
            /* get the type of digest string */
            if (strlen(pentry->digest) == (MD5_DIGEST_LENGTH * 2)) {
                type = MD5SUM;
            } else if (strlen(pentry->digest) == (SHA_DIGEST_LENGTH * 2)) {
                type = SHA1SUM;
            } else if (strlen(pentry->digest) == (SHA256_DIGEST_LENGTH * 2)) {
                type = SHA256SUM;
            } else {
                warnx(_("unknown digest type for pattern %s: %s"), pentry->pattern, pentry->digest);
                continue;
            }

            /* get the digest */
            if (type == SHA256SUM && !strcmp(pentry->digest, checksum(file))) {
                matched = true;
                allowed = pentry->allowed;
            } else {
                digest = compute_checksum(file->fullpath, &file->st.st_mode, type);

                if (!strcmp(pentry->digest, digest)) {
                    matched = true;
                    allowed = pentry->allowed;
                }

                free(digest);
            }
        }
    }

    /* report */
    if (matched) {
        /* use the package name for reporting */
        name = headerGetString(file->rpm_header, RPMTAG_NAME);

        /* initialize reporting parameters */
        init_result_params(&params);
        params.arch = get_rpm_header_arch(file->rpm_header);
        params.file = file->localpath;
        params.header = HEADER_POLITICS;
        params.remedy = REMEDY_POLITICS;

        if (allowed) {
            xasprintf(&params.msg, _("Possible politically sensitive file (%s) found in %s on %s: rules allow this file."), file->localpath, name, params.arch);
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
        } else {
            xasprintf(&params.msg, _("Possible politically sensitive file (%s) found in %s on %s: rules prohibit this file."), file->localpath, name, params.arch);
            params.severity = RESULT_BAD;
            params.waiverauth = NOT_WAIVABLE;
            params.verb = VERB_FAILED;
            params.noun = _("${FILE} is politically sensitive");
            result = false;
        }

        add_result(ri, &params);
        free(params.msg);
    }

    return result;
}

bool inspect_politics(struct rpminspect *ri)
{
    bool result = true;
    struct result_params params;

    assert(ri != NULL);

    /* run the politics check on each file */
    if (init_politics(ri)) {
        result = foreach_peer_file(ri, politics_driver, false);
    }

    /* hope the result is always this */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_POLITICS;
        add_result(ri, &params);
    }

    return result;
}
