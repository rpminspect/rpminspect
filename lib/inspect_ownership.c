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
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>
#include <err.h>
#include <sys/capability.h>

#include "rpminspect.h"

/*
 * Given an RPM header tag, get that header tag array and return the
 * string that matches the index value for this file.  That's complex,
 * but some tags are arrays of strings (or ints) and what we need to
 * do is first get the array, then knowing the index entry for the file
 * we have, pull that array index out and return it.
 *
 * Limitations:
 * "tag" must refer to an s[] tag (see rpmtag.h from librpm)
 * "file" must have a usable array index value (idx)
 *
 * Returned value must be free'd by caller.
 */
static char *get_header_value(const rpmfile_entry_t *file, rpmTag tag)
{
    rpmtd td = NULL;
    rpmFlags flags = HEADERGET_MINMEM | HEADERGET_EXT | HEADERGET_ARGV;
    const char *val = NULL;
    char *ret = NULL;

    assert(file != NULL);
    assert(file->idx >= 0);

    /* new header transaction */
    td = rpmtdNew();

    /* find the header tag we want to extract values from */
    if (!headerGet(file->rpm_header, tag, td, flags)) {
        warn(_("*** unable to find tag %s for %s"), rpmTagGetName(tag), file->fullpath);
        rpmtdFree(td);
        return ret;
    }

    /* set the array index */
    if (rpmtdSetIndex(td, file->idx) == -1) {
        warn(_("*** file index %d is out of bounds for %s"), file->idx, file->fullpath);
        rpmtdFree(td);
        return ret;
    }

    /* get the tag we are looking for and copy the value */
    val = rpmtdGetString(td);
    assert(val != NULL);
    ret = strdup(val);
    rpmtdFree(td);

    return ret;
}

/* Main driver for the 'ownership' inspection */
static bool ownership_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    bool result = true;
    const char *arch = NULL;
    char *owner = NULL;
    char *group = NULL;
    char *before_owner = NULL;
    char *before_group = NULL;
    string_entry_t *entry = NULL;
    bool bin = false;
    char *before_val = NULL;
    char *after_val = NULL;
    char *what = NULL;
    cap_t cap = NULL;
    cap_flag_value_t have_setuid = CAP_CLEAR;
    struct result_params params;

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Get the arch, we'll use that */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Get the owner and group of the file */
    owner = get_header_value(file, RPMTAG_FILEUSERNAME);
    group = get_header_value(file, RPMTAG_FILEGROUPNAME);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_OWNERSHIP;
    params.arch = arch;
    params.file = file->localpath;

    /*
     * AFTER ONLY
     */

    /* Report forbidden file owners */
    if (ri->forbidden_owners && list_contains(ri->forbidden_owners, owner) && (ri->tests & INSPECT_OWNERSHIP)) {
        xasprintf(&params.msg, _("File %s has forbidden owner `%s` on %s"), file->localpath, owner, arch);
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.remedy = REMEDY_OWNERSHIP_DEFATTR;
        add_result(ri, &params);
        free(params.msg);
        result = false;
    }

    /* Report forbidden file groups */
    if (ri->forbidden_groups && list_contains(ri->forbidden_groups, group) && (ri->tests & INSPECT_OWNERSHIP)) {
        xasprintf(&params.msg, _("File %s has forbidden group `%s` on %s"), file->localpath, owner, arch);
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.remedy = REMEDY_OWNERSHIP_DEFATTR;
        add_result(ri, &params);
        free(params.msg);
        result = false;
    }

    /* Report files in bin paths not under the bin owner or group */
    TAILQ_FOREACH(entry, ri->bin_paths, items) {
        if (strprefix(file->localpath, entry->data)) {
            bin = true;

            /* Check the owner */
            if (strcmp(owner, ri->bin_owner) && !match_fileinfo_owner(ri, file, owner, NAME_OWNERSHIP, NULL) && (ri->tests & INSPECT_OWNERSHIP)) {
                xasprintf(&params.msg, _("File %s has owner `%s` on %s, but should be `%s`"), file->localpath, owner, arch, ri->bin_owner);
                params.severity = RESULT_BAD;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.remedy = REMEDY_OWNERSHIP_BIN_OWNER;
                add_result(ri, &params);
                free(params.msg);
                result = false;
            }

            /* Check the group - special handling */
            if (strcmp(group, ri->bin_group)) {
                /* Gather capabilities(7) for the file we need */
                cap = get_cap(file);

                if (cap) {
                    if (cap_get_flag(cap, CAP_SETUID, CAP_EFFECTIVE, &have_setuid) == -1) {
                        warnx("cap_get_flag()");
                        have_setuid = CAP_CLEAR;
                    }
                }

                /* Handle if CAP_SETUID is present or not */
                if (have_setuid == CAP_SET) {
                    if (file->st.st_mode & S_IXOTH & (ri->tests & INSPECT_OWNERSHIP)) {
                        xasprintf(&params.msg, _("File %s on %s has CAP_SETUID capability but group `%s` and is world executable"), file->localpath, arch, group);
                        params.severity = RESULT_BAD;
                        params.waiverauth = WAIVABLE_BY_ANYONE;
                        params.remedy = REMEDY_OWNERSHIP_IXOTH;
                        add_result(ri, &params);
                        free(params.msg);
                        result = false;
                    }

                    if (file->st.st_mode & S_IWGRP) {
                        xasprintf(&params.msg, _("File %s on %s has CAP_SETUID capability but group `%s` and is group writable"), file->localpath, arch, group);
                        params.severity = RESULT_BAD;
                        params.waiverauth = WAIVABLE_BY_SECURITY;
                        params.remedy = REMEDY_OWNERSHIP_IWGRP;
                        add_result(ri, &params);
                        free(params.msg);
                        result = false;
                    }
                } else if (!match_fileinfo_group(ri, file, group, NAME_OWNERSHIP, NULL) && (ri->tests & INSPECT_OWNERSHIP)) {
                    xasprintf(&params.msg, _("File %s has group `%s` on %s, but should be `%s`"), file->localpath, group, arch, ri->bin_group);
                    params.severity = RESULT_BAD;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
                    params.remedy = REMEDY_OWNERSHIP_BIN_GROUP;
                    add_result(ri, &params);
                    free(params.msg);
                    result = false;
                }
            }

            break;
        }
    }

    /*
     * BEFORE AND AFTER
     */
    if (file->peer_file && (ri->tests & INSPECT_OWNERSHIP)) {
        /* Get the before file values */
        before_owner = get_header_value(file->peer_file, RPMTAG_FILEUSERNAME);
        before_group = get_header_value(file->peer_file, RPMTAG_FILEGROUPNAME);

        /* Determine if anything changed */
        if (strcmp(before_owner, owner)) {
            what = "owner";
            before_val = strdup(before_owner);
            after_val = strdup(owner);
        }

        if (strcmp(before_group, group)) {
            if (what) {
                what = "owner:group";
                free(before_val);
                free(after_val);
                xasprintf(&before_val, "%s:%s", before_owner, before_group);
                xasprintf(&after_val, "%s:%s", owner, group);
            } else {
                what = "group";
                before_val = strdup(before_group);
                after_val = strdup(group);
            }
        }

        /* Report out changes, if necessary */
        if (what) {
            /* Change the severity depending on what happened */
            xasprintf(&params.msg, "%s:%s", owner, group);
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;

            if (bin &&
                ((!strcmp(owner, ri->bin_owner) && !strcmp(what, "owner")) ||
                 (!strcmp(group, ri->bin_group) && !strcmp(what, "group")) ||
                 (!strcmp(params.msg, after_val) && !strcmp(what, "owner:group")))) {
                /*
                 * If ownership changed to bin_owner/bin_group for bin files,
                 * just make it an informational message.
                 */
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
            }

            free(params.msg);
            xasprintf(&params.msg, _("File %s changed %s from `%s` to `%s` on %s"), file->localpath, what, before_val, after_val, arch);
            params.remedy = REMEDY_OWNERSHIP_CHANGED;
            add_result(ri, &params);
            free(params.msg);
            result = false;
        }

        free(before_owner);
        free(before_group);
        free(before_val);
        free(after_val);
    }

    free(owner);
    free(group);
    return result;
}

/*
 * Main driver for the 'ownership' inspection.
 */
bool inspect_ownership(struct rpminspect *ri) {
    bool result = false;
    struct result_params params;

    assert(ri != NULL);
    result = foreach_peer_file(ri, NAME_OWNERSHIP, ownership_driver, true);

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_OWNERSHIP;
        add_result(ri, &params);
    }

    return result;
}
