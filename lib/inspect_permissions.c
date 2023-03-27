/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool reported = false;

static bool permissions_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    bool ignore = false;
    const char *arch = NULL;
    char *change = NULL;
    mode_t before_mode;
    mode_t after_mode;
    mode_t mode_diff;
    bool allowed = false;
    char *noun = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Ignore files in the SRPM */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Ignore debuginfo and debugsource packages */
    if (strprefix(file->localpath, DEBUG_PATH) || strprefix(file->localpath, DEBUG_SRC_PATH)) {
        return true;
    }

    /* We will skip checks for ignored files */
    ignore = ignore_rpmfile_entry(ri, NAME_PERMISSIONS, file);

    /* We need the architecture for reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_PERMISSIONS;
    params.arch = arch;
    params.file = file->localpath;

    /* Local working copies for display */
    after_mode = file->st.st_mode & 0777;

    /* Compare the modes */
    if (file->peer_file) {
        before_mode = file->peer_file->st.st_mode & 0777;
    } else {
        memset(&before_mode, 0, sizeof(before_mode));
    }

    mode_diff = before_mode ^ after_mode;
    allowed = match_fileinfo_mode(ri, file, NAME_PERMISSIONS, NULL, &result, &reported);

    /* if setuid/setgid or new mode is more open */
    if (!ignore && mode_diff && file->peer_file && !allowed && (ri->tests & INSPECT_PERMISSIONS)) {
        params.msg = strdup(file->localpath);
        assert(params.msg != NULL);

        if (!(before_mode & (S_ISUID|S_ISGID)) && (after_mode & (S_ISUID|S_ISGID))) {
            params.severity = RESULT_VERIFY;
            params.msg = strappend(params.msg, _(" changed setuid/setgid;"));
            assert(params.msg != NULL);
        }

        if (S_ISDIR(file->st.st_mode) && !S_ISDIR(file->peer_file->st.st_mode)) {
            params.msg = strappend(params.msg, _(" became a directory;"), NULL);
        } else if (S_ISCHR(file->st.st_mode) && !S_ISCHR(file->peer_file->st.st_mode)) {
            params.msg = strappend(params.msg, _(" became a character device;"), NULL);
        } else if (S_ISBLK(file->st.st_mode) && !S_ISBLK(file->peer_file->st.st_mode)) {
            params.msg = strappend(params.msg, _(" became a block device;"), NULL);
        } else if (S_ISREG(file->st.st_mode) && !S_ISREG(file->peer_file->st.st_mode)) {
            params.msg = strappend(params.msg, _(" became a regular file;"), NULL);
        } else if (S_ISFIFO(file->st.st_mode) && !S_ISFIFO(file->peer_file->st.st_mode)) {
            params.msg = strappend(params.msg, _(" became a FIFO;"), NULL);
        } else if (S_ISLNK(file->st.st_mode) && !S_ISLNK(file->peer_file->st.st_mode)) {
            params.msg = strappend(params.msg, _(" became a symbolic link;"), NULL);
        } else if (S_ISSOCK(file->st.st_mode) && !S_ISSOCK(file->peer_file->st.st_mode)) {
            params.msg = strappend(params.msg, _(" became a socket;"), NULL);
        }

        assert(params.msg != NULL);
        params.msg = strappend(params.msg, _(" permissions"), NULL);
        assert(params.msg != NULL);

        if (S_ISDIR(file->st.st_mode) && !S_ISLNK(file->st.st_mode)) {
            if (((mode_diff & S_ISVTX) && !(after_mode & S_ISVTX)) || ((after_mode & mode_diff) != 0)) {
                params.severity = RESULT_VERIFY;
                params.msg = strappend(params.msg, _(" relaxed"), NULL);
            }
        } else {
            params.msg = strappend(params.msg, _(" changed"), NULL);
        }

        assert(params.msg != NULL);

        if (params.severity >= RESULT_VERIFY) {
            params.waiverauth = WAIVABLE_BY_ANYONE;
            result = false;
        }

        xasprintf(&change, _("%s from %04o to %04o on %s"), params.msg, before_mode, after_mode, arch);
        assert(change != NULL);
        free(params.msg);
        params.msg = change;

        params.verb = VERB_CHANGED;
        xasprintf(&noun, _("${FILE} permissions from %04o to %04o on ${ARCH}"), before_mode, after_mode);
        params.noun = noun;
        add_result(ri, &params);
        free(params.msg);
        free(noun);

        reported = true;
    }

    /* check for world-writability */
    if (!allowed && !S_ISLNK(file->st.st_mode) && ((after_mode & (S_IWOTH|S_ISVTX)) || (after_mode & S_IWOTH))) {
        params.severity = get_secrule_result_severity(ri, file, SECRULE_WORLDWRITABLE);

        if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
            xasprintf(&params.msg, _("%s (%s) is world-writable on %s"), file->localpath, get_mime_type(file), arch);
            params.waiverauth = WAIVABLE_BY_SECURITY;
            params.verb = VERB_FAILED;
            params.noun = _("${FILE} is world-writable on ${ARCH}");
            add_result(ri, &params);
            free(params.msg);

            result = false;
            reported = true;
        }
    }

    return result;
}

/*
 * Main driver for the 'permissions' inspection.
 */
bool inspect_permissions(struct rpminspect *ri)
{
    bool result = false;
    struct result_params params;

    assert(ri != NULL);

    /* run the permissions inspection across all RPM files */
    result = foreach_peer_file(ri, NAME_PERMISSIONS, permissions_driver);

    /* if everything was fine, just say so */
    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_PERMISSIONS;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
