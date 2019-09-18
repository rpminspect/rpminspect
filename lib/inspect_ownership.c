/*
 * Copyright (C) 2019  Red Hat, Inc.
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
#include <cap-ng.h>

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
        fprintf(stderr, "*** unable to find tag %d for %s\n", tag, file->fullpath);
        abort();
    }

    /* walk the tag and cram everything in to a list */
    while ((val = rpmtdNextString(td)) != NULL) {
        if (rpmtdGetIndex(td) == file->idx) {
            ret = strdup(val);
            break;
        }
    }

    rpmtdFreeData(td);
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
    char *msg = NULL;
    string_entry_t *entry = NULL;
    severity_t sev;
    bool bin = false;
    char *before_val = NULL;
    char *after_val = NULL;
    char *what = NULL;
    int fd = -1;
    int cap = -1;

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Get the arch, we'll use that */
    arch = headerGetString(file->rpm_header, RPMTAG_ARCH);

    /* Get the owner and group of the file */
    owner = get_header_value(file, RPMTAG_FILEUSERNAME);
    group = get_header_value(file, RPMTAG_FILEGROUPNAME);

    /*
     * AFTER ONLY
     */

    /* Report forbidden file owners */
    if (ri->forbidden_owners) {
        TAILQ_FOREACH(entry, ri->forbidden_owners, items) {
            if (!strcmp(owner, entry->data)) {
                xasprintf(&msg, "File %s has forbidden owner `%s` on %s", file->localpath, owner, arch);
                add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_OWNERSHIP, msg, NULL, REMEDY_OWNERSHIP_DEFATTR);
                free(msg);
                result = false;
                break;
            }
        }
    }

    /* Report forbidden file groups */
    if (ri->forbidden_groups) {
        TAILQ_FOREACH(entry, ri->forbidden_groups, items) {
            if (!strcmp(group, entry->data)) {
                xasprintf(&msg, "File %s has forbidden group `%s` on %s", file->localpath, owner, arch);
                add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_OWNERSHIP, msg, NULL, REMEDY_OWNERSHIP_DEFATTR);
                free(msg);
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
            if (strcmp(owner, ri->bin_owner)) {
                xasprintf(&msg, "File %s has owner `%s` on %s, but should be `%s`", file->localpath, owner, arch, ri->bin_owner);
                add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_OWNERSHIP, msg, NULL, REMEDY_OWNERSHIP_BIN_OWNER);
                free(msg);
                result = false;
            }

            /* Check the group - special handling */
            if (strcmp(group, ri->bin_group)) {
                /* Gather capabilities(7) for the file we need */
                if ((fd = open(file->fullpath, O_RDONLY)) == -1) {
                    fprintf(stderr, "*** unable to open() %s on %s: %s\n", file->localpath, arch, strerror(errno));
                    break;
                }

                cap = capng_get_caps_fd(fd);

                if (close(fd) == -1) {
                    fprintf(stderr, "*** unable to close() %s on %s: %s\n", file->localpath, arch, strerror(errno));
                    break;
                }

                /* Handle if CAP_SETUID is present or not */
                if ((cap == 0) && capng_have_capability(CAPNG_EFFECTIVE, CAP_SETUID)) {
                    if (file->st.st_mode & S_IXOTH) {
                        xasprintf(&msg, "File %s on %s has CAP_SETUID capability but group `%s` and is world executable", file->localpath, arch, group);
                        add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_OWNERSHIP, msg, NULL, REMEDY_OWNERSHIP_IXOTH);
                        free(msg);
                        result = false;
                    }

                    if (file->st.st_mode & S_IWGRP) {
                        xasprintf(&msg, "File %s on %s has CAP_SETUID capability but group `%s` and is group writable", file->localpath, arch, group);
                        add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_SECURITY, HEADER_OWNERSHIP, msg, NULL, REMEDY_OWNERSHIP_IWGRP);
                        free(msg);
                        result = false;
                    }
                } else {
                    xasprintf(&msg, "File %s has group `%s` on %s, but should be `%s`", file->localpath, group, arch, ri->bin_group);
                    add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_OWNERSHIP, msg, NULL, REMEDY_OWNERSHIP_BIN_GROUP);
                    free(msg);
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
            xasprintf(&msg, "%s:%s", owner, group);
            sev = RESULT_VERIFY;

            if (bin &&
                ((!strcmp(owner, ri->bin_owner) && !strcmp(what, "owner")) ||
                 (!strcmp(group, ri->bin_group) && !strcmp(what, "group")) ||
                 (!strcmp(msg, after_val) && !strcmp(what, "owner:group")))) {
                /*
                 * If ownership changed to bin_owner/bin_group for bin files,
                 * just make it an informational message.
                 */
                sev = RESULT_INFO;
            }

            free(msg);
            xasprintf(&msg, "File %s changed %s from `%s` to `%s` on %s", file->localpath, what, before_val, after_val, arch);
            add_result(&ri->results, sev, WAIVABLE_BY_ANYONE, HEADER_OWNERSHIP, msg, NULL, REMEDY_OWNERSHIP_CHANGED);
            free(msg);
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

    assert(ri != NULL);
    result = foreach_peer_file(ri, ownership_driver);

    if (result) {
        add_result(&ri->results, RESULT_OK, NOT_WAIVABLE, HEADER_OWNERSHIP, NULL, NULL, NULL);
    }

    return result;
}
