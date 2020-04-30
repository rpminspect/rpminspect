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

#include <assert.h>
#include "rpminspect.h"

/*
 * Check for the given path on the stat-whitelist.  Report accordingly.
 * Returns true if the path is on the whitelist, false if it isn't.
 */
bool on_stat_whitelist(struct rpminspect *ri, const rpmfile_entry_t *file, const char *header, const char *remedy)
{
    stat_whitelist_entry_t *wlentry = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    init_result_params(&params);
    params.header = header;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;
    params.remedy = remedy;

    if (init_stat_whitelist(ri)) {
        TAILQ_FOREACH(wlentry, ri->stat_whitelist, items) {
            if (!strcmp(file->localpath, wlentry->filename)) {
                if (file->st.st_mode == wlentry->mode) {
                    xasprintf(&params.msg, _("%s on %s carries mode %04o, but is on the stat whitelist"), file->localpath, params.arch, file->st.st_mode);
                    params.severity = RESULT_INFO;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
                    add_result(ri, &params);
                    free(params.msg);
                    return true;
                } else {
                    xasprintf(&params.msg, _("%s on %s carries mode %04o, is on the stat whitelist but expected mode %04o"), file->localpath, params.arch, file->st.st_mode, wlentry->mode);
                    params.severity = RESULT_VERIFY;
                    params.waiverauth = WAIVABLE_BY_SECURITY;
                    add_result(ri, &params);
                    free(params.msg);
                    return true;
                }
            }
        }
    }

    /* catch anything not on the stat-whitelist */
    xasprintf(&params.msg, _("%s on %s carries insecure mode %04o, Security Team review may be required"), file->localpath, params.arch, file->st.st_mode);
    params.severity = RESULT_BAD;
    params.waiverauth = WAIVABLE_BY_SECURITY;
    add_result(ri, &params);
    free(params.msg);
    return true;
}

/*
 * Return the caps_whitelist entry that matches the package and filepath.  If
 * it doesn't exist on the list, return NULL.  This function will take care of
 * initializing the caps_whitelist if necessary.
 */
caps_filelist_entry_t *get_caps_whitelist_entry(struct rpminspect *ri, const char *pkg, const char *filepath)
{
    caps_whitelist_entry_t *wlentry = NULL;
    caps_filelist_entry_t *flentry = NULL;

    assert(ri != NULL);
    assert(pkg != NULL);
    assert(filepath != NULL);

    if (init_caps_whitelist(ri)) {
        /* Look for the package in the caps whitelist */
        TAILQ_FOREACH(wlentry, ri->caps_whitelist, items) {
            if (!strcmp(wlentry->pkg, pkg)) {
                break;
            }
        }

        if (wlentry == NULL) {
            return NULL;
        }

        /* Look for this file's entry for that package */
        TAILQ_FOREACH(flentry, wlentry->files, items) {
            if (strsuffix(flentry->path, filepath)) {
                break;
            }
        }

        /* No entry, make sure to return NULL */
        if (flentry == NULL) {
            return NULL;
        }
    }

    return flentry;
}
