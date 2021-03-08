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

#include "rpminspect.h"

static bool annocheck_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    char *cmd = NULL;
    string_map_t *hentry = NULL;
    string_map_t *tmp_hentry = NULL;
    char *wrkdir = NULL;
    size_t fl = 0;
    size_t ll = 0;
    char *tmp_out = NULL;
    char *after_out = NULL;
    int after_exit;
    char *before_out = NULL;
    int before_exit;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Ignore files in the SRPM */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Ignore debug and build paths */
    if (is_debug_or_build_path(file->localpath)) {
        return true;
    }

    /* We need the architecture for reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Only run this check on ELF files */
    if (!is_elf(file->fullpath) || (!is_elf(file->fullpath) && file->peer_file && !is_elf(file->peer_file->fullpath))) {
        return result;
    }

    /* Set up the result parameters */
    init_result_params(&params);
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.header = HEADER_ANNOCHECK;
    params.remedy = REMEDY_ANNOCHECK;
    params.arch = arch;
    params.file = file->localpath;

    /* Run each annocheck test and report the results */
    HASH_ITER(hh, ri->annocheck, hentry, tmp_hentry) {
        /* Run the test on the file */
        if (get_after_debuginfo_path(ri, arch) == NULL) {
            xasprintf(&cmd, "%s %s %s", ri->commands.annocheck, hentry->value, file->fullpath);
        } else {
            xasprintf(&cmd, "%s %s --debug-dir=%s %s", ri->commands.annocheck, hentry->value, get_after_debuginfo_path(ri, arch), file->fullpath);
        }

        DEBUG_PRINT("%s\n", cmd);
        after_out = run_cmd(&after_exit, cmd, NULL);
        free(cmd);

        /* If we have a before build, run the command on that */
        if (file->peer_file) {
            if (get_before_debuginfo_path(ri, arch) == NULL) {
                xasprintf(&cmd, "%s %s %s", ri->commands.annocheck, hentry->value, file->peer_file->fullpath);
            } else {
                xasprintf(&cmd, "%s %s --debug-dir=%s %s", ri->commands.annocheck, hentry->value, get_before_debuginfo_path(ri, arch), file->peer_file->fullpath);
            }

            DEBUG_PRINT("%s\n", cmd);
            before_out = run_cmd(&before_exit, cmd, NULL);
            free(cmd);
        }

        /* Build a reporting message if we need to */
        if (before_out && after_out) {
            if (before_exit == 0 && after_exit == 0) {
                xasprintf(&params.msg, _("annocheck '%s' test passes for %s on %s"), hentry->key, file->localpath, arch);
            } else if (before_exit == 1 && after_exit == 0) {
                xasprintf(&params.msg, _("annocheck '%s' test now passes for %s on %s"), hentry->key, file->localpath, arch);
            } else if (before_exit == 0 && after_exit == 1) {
                xasprintf(&params.msg, _("annocheck '%s' test now fails for %s on %s"), hentry->key, file->localpath, arch);
                params.severity = RESULT_VERIFY;
                params.verb = VERB_CHANGED;
                result = false;
            }
        } else if (after_out) {
            if (after_exit == 0) {
                xasprintf(&params.msg, _("annocheck '%s' test passes for %s on %s"), hentry->key, file->localpath, arch);
            } else if (after_exit == 1) {
                xasprintf(&params.msg, _("annocheck '%s' test fails for %s on %s"), hentry->key, file->localpath, arch);
                params.severity = RESULT_VERIFY;
                params.verb = VERB_CHANGED;
                result = false;
            }
        }

        /* Report the results */
        if (params.msg) {
            /* trim the working directory from the details if it exists */
            fl = strlen(file->fullpath);
            ll = strlen(file->localpath);

            if (fl > ll) {
                wrkdir = strndup(file->fullpath, fl - ll);
            }

            if (wrkdir) {
                tmp_out = strreplace(after_out, wrkdir, NULL);
                free(after_out);
                free(wrkdir);
                after_out = tmp_out;
            }

            params.details = after_out;
            add_result(ri, &params);
            free(params.msg);
        }

        /* Cleanup */
        free(after_out);
        free(before_out);
        after_out = NULL;
        before_out = NULL;
    }

    return result;
}

/*
 * Main driver for the 'annocheck' inspection.
 */
bool inspect_annocheck(struct rpminspect *ri) {
    bool result;
    struct result_params params;

    assert(ri != NULL);

    /* skip if we have no annocheck tests defined */
    if (ri->annocheck == NULL) {
        return true;
    }

    /* run the annocheck tests across all ELF files */
    result = foreach_peer_file(ri, annocheck_driver, true);

    /* if everything was fine, just say so */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_ANNOCHECK;
        add_result(ri, &params);
    }

    return result;
}
