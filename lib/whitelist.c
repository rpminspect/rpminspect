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

#include <assert.h>
#include "rpminspect.h"

/*
 * Check for the given path on the stat-whitelist.  Report accordingly.
 * Returns true if the path is on the whitelist, false if it isn't.
 */
bool on_stat_whitelist(struct rpminspect *ri, const rpmfile_entry_t *file, const char *header, const char *remedy)
{
    const char *arch = NULL;
    stat_whitelist_entry_t *wlentry = NULL;
    char *msg = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    arch = get_rpm_header_arch(file->rpm_header);

    if (init_stat_whitelist(ri)) {
        TAILQ_FOREACH(wlentry, ri->stat_whitelist, items) {
            if (!strcmp(file->localpath, wlentry->filename)) {
                if (file->st.st_mode == wlentry->mode) {
                    xasprintf(&msg, "%s on %s carries mode %04o, but is on the stat whitelist", file->localpath, arch, file->st.st_mode);
                    add_result(ri, RESULT_INFO, WAIVABLE_BY_ANYONE, header, msg, NULL, remedy);
                    free(msg);
                    return true;
                } else {
                    xasprintf(&msg, "%s on %s carries mode %04o, is on the stat whitelist but expected mode %04o", file->localpath, arch, file->st.st_mode, wlentry->mode);
                    add_result(ri, RESULT_VERIFY, WAIVABLE_BY_SECURITY, header, msg, NULL, remedy);
                    free(msg);
                    return true;
                }
            }
        }
    }

    /* catch anything not on the stat-whitelist */
    xasprintf(&msg, "%s on %s carries insecure mode %04o, Security Team review may be required", file->localpath, arch, file->st.st_mode);
    add_result(ri, RESULT_BAD, WAIVABLE_BY_SECURITY, header, msg, NULL, remedy);
    free(msg);
    return true;
}
