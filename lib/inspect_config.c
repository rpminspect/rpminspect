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

static bool reported = false;

static bool config_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    rpmfileAttrs before_config = 0;
    rpmfileAttrs after_config = 0;
    ssize_t n = 0;
    char before_dest[PATH_MAX + 1];
    char after_dest[PATH_MAX + 1];
    char *diff_output = NULL;
    int exitcode;
    const char *name = NULL;
    const char *arch = NULL;
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

    /* only compare regular files and symlinks */
    if (S_ISDIR(file->st.st_mode) || S_ISCHR(file->st.st_mode) || S_ISBLK(file->st.st_mode) || S_ISFIFO(file->st.st_mode) || S_ISSOCK(file->st.st_mode)) {
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
    params.header = HEADER_CONFIG;
    params.arch = arch;
    params.file = file->localpath;
    params.remedy = REMEDY_CONFIG;
    params.verb = VERB_CHANGED;
    params.noun = _("%config ${FILE}");

    if (is_rebase(ri)) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
    } else {
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
    }

    /* verify %config values */
    before_config |= file->peer_file->flags & RPMFILE_CONFIG;
    after_config |= file->flags & RPMFILE_CONFIG;

    if (before_config && after_config) {
        /*
         * according to legend, this can only really work reliably for
         * reporting %config entries that change to/from being
         * symlinks, but not actually link destination comparisons.
         * That makes sense because link destinations can really only
         * be compared if they are relative to the package root so
         * resolution works.  Absolutel symlinks will fail this
         * routine.
         */
        if (S_ISLNK(file->st.st_mode) || S_ISLNK(file->peer_file->st.st_mode)) {
            /* read the before link destination */
            if (S_ISLNK(file->peer_file->st.st_mode)) {
                memset(before_dest, '\0', sizeof(before_dest));
                n = readlink(file->peer_file->fullpath, before_dest, PATH_MAX);

                if (n == -1) {
                    warn("readlink(%s)", file->peer_file->fullpath);
                    return true;
                }
            }

            /* read the after link destination */
            if (S_ISLNK(file->st.st_mode)) {
                memset(after_dest, '\0', sizeof(after_dest));
                n = readlink(file->fullpath, after_dest, PATH_MAX);

                if (n == -1) {
                    warn("readlink(%s)", file->fullpath);
                    return true;
                }
            }

            /* report changes */
            if (!S_ISLNK(file->peer_file->st.st_mode) && S_ISLNK(file->st.st_mode)) {
                xasprintf(&params.msg, _("%%config file %s went from actual file to symlink (pointing to %s) in %s on %s"), file->localpath, after_dest, name, arch);
                add_result(ri, &params);
                free(params.msg);
                reported = true;

                if (params.severity == RESULT_VERIFY) {
                    result = false;
                }
            } else if (S_ISLNK(file->peer_file->st.st_mode) && !S_ISLNK(file->st.st_mode)) {
                xasprintf(&params.msg, _("%%config file %s was a symlink (pointing to %s), became an actual file in %s on %s"), file->peer_file->localpath, before_dest, name, arch);
                add_result(ri, &params);
                free(params.msg);
                reported = true;

                if (params.severity == RESULT_VERIFY) {
                    result = false;
                }
            } else if (strcmp(before_dest, after_dest)) {
                xasprintf(&params.msg, _("Symlink value for %%config file %s changed in %s on %s."), file->localpath, name, arch);
                xasprintf(&params.details, "  - %s\n  + %s\n", before_dest, after_dest);
                add_result(ri, &params);
                free(params.msg);
                free(params.details);
                reported = true;

                if (params.severity == RESULT_VERIFY) {
                    result = false;
                }
            }
        } else {
            /* compare the files */
            exitcode = filecmp(file->peer_file->fullpath, file->fullpath);

            if (exitcode) {
                /* the files differ and not a rebase, see if it's only whitespace changes */
                free(diff_output);
                params.details = run_cmd(&exitcode, DIFF_CMD, "-u", "-w", "-I^#.*", file->peer_file->fullpath, file->fullpath, NULL);

                if (exitcode == 0) {
                    xasprintf(&params.msg, _("%%config file content change for %s in %s on %s (comments/whitespace only)"), file->localpath, name, arch);
                    params.severity = RESULT_INFO;
                    params.waiverauth = NOT_WAIVABLE;
                    result = true;
                } else {
                    xasprintf(&params.msg, _("%%config file content change for %s in %s on %s"), file->localpath, name, arch);

                    if (params.severity == RESULT_VERIFY) {
                        result = false;
                    }
                }

                add_result(ri, &params);
                free(params.msg);
                free(params.details);
                reported = true;
            }
        }
    } else if (before_config || after_config) {
        xasprintf(&params.msg, _("%%config file change for %s in %s on %s (%smarked as %%config -> %smarked as %%config)\n"), file->localpath, name, arch,
                  (before_config ? "" : _("not ")), (after_config ? "" : _("not ")));
        add_result(ri, &params);
        free(params.msg);
        result = false;
        reported = true;
    }

    return result;
}

/*
 * Main driver for the 'config' inspection.
 */
bool inspect_config(struct rpminspect *ri) {
    bool result = true;
    struct result_params params;

    assert(ri != NULL);

    result = foreach_peer_file(ri, config_driver, true);

    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_CONFIG;
        add_result(ri, &params);
    }

    return result;
}
