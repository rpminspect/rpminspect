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
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
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

    /* get the tag */
    td = rpmtdNew();

    if (!headerGet(file->rpm_header, tag, td, flags)) {
        fprintf(stderr, _("*** unable to find tag %d for %s\n"), tag, file->fullpath);
        abort();
    }

    /* walk the tag and cram everything in to a list */
    while ((val = rpmtdNextString(td)) != NULL) {
        if (rpmtdGetIndex(td) == file->idx) {
            ret = strdup(val);
            break;
        }
    }

    rpmtdFree(td);
    return ret;
}

/* Main driver for the 'ownership' inspection */
static bool ownership_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    bool result = true;
    const char *arch = NULL;
    char *owner = NULL;
    char *group = NULL;
    struct passwd pw;
    struct passwd *pwp = NULL;
    char pbuf[sysconf(_SC_GETPW_R_SIZE_MAX)];
    struct group gr;
    struct group *grp = NULL;
    char gbuf[sysconf(_SC_GETGR_R_SIZE_MAX)];
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

    /*
     * Look up the ID values of the owner and name and put those in
     * the struct stat
     */
    if (getpwnam_r(owner, &pw, pbuf, sizeof(pbuf), &pwp)) {
        err(2, "getpwnam_r");
    }

    if (getgrnam_r(group, &gr, gbuf, sizeof(gbuf), &grp)) {
        err(2, "getgrnam_r");
    }

    file->st.st_uid = pw.pw_uid;
    file->st.st_gid = gr.gr_gid;

    /* Set up result parameters */
    init_result_params(&params);
    params.header = HEADER_OWNERSHIP;
    params.arch = arch;
    params.file = file->localpath;

    /*
     * AFTER ONLY
     */

    /* Report forbidden file owners */
    if (ri->forbidden_owners) {
        TAILQ_FOREACH(entry, ri->forbidden_owners, items) {
            if (!strcmp(owner, entry->data)) {
                xasprintf(&params.msg, _("File %s has forbidden owner `%s` on %s"), file->localpath, owner, arch);
                params.severity = RESULT_BAD;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.remedy = REMEDY_OWNERSHIP_DEFATTR;
                add_result(ri, &params);
                free(params.msg);
                result = false;
                break;
            }
        }
    }

    /* Report forbidden file groups */
    if (ri->forbidden_groups) {
        TAILQ_FOREACH(entry, ri->forbidden_groups, items) {
            if (!strcmp(group, entry->data)) {
                xasprintf(&params.msg, _("File %s has forbidden group `%s` on %s"), file->localpath, owner, arch);
                params.severity = RESULT_BAD;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.remedy = REMEDY_OWNERSHIP_DEFATTR;
                add_result(ri, &params);
                free(params.msg);
                result = false;
                break;
            }
        }
    }

    /* Report files in bin paths not under the bin owner or group */
    TAILQ_FOREACH(entry, ri->bin_paths, items) {
        if (strprefix(file->localpath, entry->data)) {
            bin = true;

            /* Check the owner */
            if (strcmp(owner, ri->bin_owner) && !on_stat_whitelist_owner(ri, file, owner, HEADER_OWNERSHIP, NULL)) {
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
                        fprintf(stderr, _("*** unable to get capabilities for %s\n"), file->localpath);
                        have_setuid = CAP_CLEAR;
                    }
                }

                /* Handle if CAP_SETUID is present or not */
                if (have_setuid == CAP_SET) {
                    if (file->st.st_mode & S_IXOTH) {
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
                } else if (!on_stat_whitelist_group(ri, file, group, HEADER_OWNERSHIP, NULL)) {
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
    if (file->peer_file) {
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

            if (bin &&
                ((!strcmp(owner, ri->bin_owner) && !strcmp(what, "owner")) ||
                 (!strcmp(group, ri->bin_group) && !strcmp(what, "group")) ||
                 (!strcmp(params.msg, after_val) && !strcmp(what, "owner:group")))) {
                /*
                 * If ownership changed to bin_owner/bin_group for bin files,
                 * just make it an informational message.
                 */
                params.severity = RESULT_INFO;
            }

            free(params.msg);
            xasprintf(&params.msg, _("File %s changed %s from `%s` to `%s` on %s"), file->localpath, what, before_val, after_val, arch);
            params.waiverauth = WAIVABLE_BY_ANYONE;
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
    result = foreach_peer_file(ri, ownership_driver);

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_OWNERSHIP;
        add_result(ri, &params);
    }

    return result;
}
