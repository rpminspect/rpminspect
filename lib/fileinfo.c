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

#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include "rpminspect.h"

/**
 * @brief Check for the given path on the fileinfo list.  If found,
 * check the st_mode value and report accordingly.
 *
 * @param ri The main struct rpminspect for the program.
 * @param file The file to find on the fileinfo list.
 * @param header The header string to use for results reporting if the
 *               file is found.
 * @param remedy The remedy string to use for results reporting if the
 *               file is found.
 * @return True if the file is on the fileinfo list, false otherwise.
 */
bool match_fileinfo_mode(struct rpminspect *ri, const rpmfile_entry_t *file, const char *header, const char *remedy)
{
    fileinfo_entry_t *fientry = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    init_result_params(&params);
    params.header = header;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;
    params.remedy = remedy;

    if (init_fileinfo(ri)) {
        TAILQ_FOREACH(fientry, ri->fileinfo, items) {
            if (!strcmp(file->localpath, fientry->filename)) {
                if (file->st.st_mode == fientry->mode) {
                    xasprintf(&params.msg, _("%s on %s carries mode %04o, but is on the fileinfo list"), file->localpath, params.arch, file->st.st_mode);
                    params.severity = RESULT_INFO;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
                    add_result(ri, &params);
                    free(params.msg);
                    return true;
                } else {
                    xasprintf(&params.msg, _("%s on %s carries mode %04o, is on the fileinfo list but expected mode %04o"), file->localpath, params.arch, file->st.st_mode, fientry->mode);
                    params.severity = RESULT_VERIFY;
                    params.waiverauth = WAIVABLE_BY_SECURITY;
                    add_result(ri, &params);
                    free(params.msg);
                    return true;
                }

                break;
            }
        }
    }

    /* catch anything not on the fileinfo list with setuid/setgid */
    if ((file->st.st_mode & S_ISUID) || (file->st.st_mode & S_ISGID)) {
        xasprintf(&params.msg, _("%s on %s carries insecure mode %04o, Security Team review may be required"), file->localpath, params.arch, file->st.st_mode);
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_SECURITY;
        add_result(ri, &params);
        free(params.msg);
    }

    return false;
}

/**
 * @brief Check for the given path on the fileinfo list.  If found,
 * check the st_uid value and report accordingly.
 *
 * @param ri The main struct rpminspect for the program.
 * @param file The file to find on the fileinfo list.
 * @param header The header string to use for results reporting if the
 *               file is found.
 * @param remedy The remedy string to use for results reporting if the
 *               file is found.
 * @return True if the file is on the fileinfo list, false otherwise.
 */
bool match_fileinfo_owner(struct rpminspect *ri, const rpmfile_entry_t *file, const char *owner, const char *header, const char *remedy)
{
    struct passwd pw;
    struct passwd *pwp = NULL;
    char buf[sysconf(_SC_GETPW_R_SIZE_MAX)];
    fileinfo_entry_t *fientry = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);
    assert(owner != NULL);

    init_result_params(&params);
    params.header = header;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;
    params.remedy = remedy;

    if (init_fileinfo(ri)) {
        TAILQ_FOREACH(fientry, ri->fileinfo, items) {
            if (!strcmp(file->localpath, fientry->filename)) {
                /* get the UID of the file on the fileinfo list */
                getpwnam_r(fientry->owner, &pw, buf, sizeof(buf), &pwp);

                if (pwp && (file->st.st_uid == pw.pw_uid) && !strcmp(owner, fientry->owner)) {
                    xasprintf(&params.msg, _("%s on %s carries owner %s (UID %d) and is on the fileinfo list"), file->localpath, params.arch, fientry->owner, file->st.st_uid);
                    params.severity = RESULT_INFO;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
                    add_result(ri, &params);
                    free(params.msg);
                    return true;
                } else if (pwp == NULL) {
                    xasprintf(&params.msg, _("%s on %s carries owner %s (UID %d) and is on the fileinfo list, but the UID cannot be verified"), file->localpath, params.arch, fientry->owner, file->st.st_uid);
                    params.severity = RESULT_VERIFY;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
                    add_result(ri, &params);
                    free(params.msg);
                    return true;
                } else {
                    xasprintf(&params.msg, _("%s on %s carries owner %s (UID %d), but is on the fileinfo list with expected owner %s (UID %d)"), file->localpath, params.arch, owner, file->st.st_uid, fientry->owner, pw.pw_uid);
                    params.severity = RESULT_VERIFY;
                    params.waiverauth = WAIVABLE_BY_SECURITY;
                    add_result(ri, &params);
                    free(params.msg);
                    return true;
                }

                break;
            }
        }
    }

    return false;
}

/**
 * @brief Check for the given path on the fileinfo list.  If found,
 * check the st_gid value and report accordingly.
 *
 * @param ri The main struct rpminspect for the program.
 * @param file The file to find on the fileinfo list.
 * @param header The header string to use for results reporting if the
 *               file is found.
 * @param remedy The remedy string to use for results reporting if the
 *               file is found.
 * @return True if the file is on the fileinfo list, false otherwise.
 */
bool match_fileinfo_group(struct rpminspect *ri, const rpmfile_entry_t *file, const char *group, const char *header, const char *remedy)
{
    struct group gr;
    struct group *grp = NULL;
    char buf[sysconf(_SC_GETGR_R_SIZE_MAX)];
    fileinfo_entry_t *fientry = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    init_result_params(&params);
    params.header = header;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;
    params.remedy = remedy;

    if (init_fileinfo(ri)) {
        TAILQ_FOREACH(fientry, ri->fileinfo, items) {
            if (!strcmp(file->localpath, fientry->filename)) {
                /* get the GID of the file on the fileinfo list */
                getgrnam_r(fientry->group, &gr, buf, sizeof(buf), &grp);

                if (grp && (file->st.st_gid == gr.gr_gid) && !strcmp(group, gr.gr_name)) {
                    xasprintf(&params.msg, _("%s on %s carries group %s (GID %d) and is on the fileinfo list"), file->localpath, params.arch, fientry->group, file->st.st_gid);
                    params.severity = RESULT_INFO;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
                    add_result(ri, &params);
                    free(params.msg);
                    return true;
                } else if (grp == NULL) {
                    xasprintf(&params.msg, _("%s on %s carries group %s (GID %d) and is on the fileinfo list, but the GID cannot be verified"), file->localpath, params.arch, fientry->group, file->st.st_gid);
                    params.severity = RESULT_VERIFY;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
                    add_result(ri, &params);
                    free(params.msg);
                    return true;
                } else {
                    xasprintf(&params.msg, _("%s on %s carries group %s (GID %d), but is on the fileinfo list with expected group %s (GID %d)"), file->localpath, params.arch, group, file->st.st_gid, fientry->group, gr.gr_gid);
                    params.severity = RESULT_VERIFY;
                    params.waiverauth = WAIVABLE_BY_SECURITY;
                    add_result(ri, &params);
                    free(params.msg);
                    return true;
                }

                break;
            }
        }
    }

    return false;
}

/*
 * Return the caps list entry that matches the package and filepath.
 * If it doesn't exist on the list, return NULL.  This function will
 * take care of initializing the caps list if necessary.
 */
caps_filelist_entry_t *get_caps_entry(struct rpminspect *ri, const char *pkg, const char *filepath)
{
    caps_entry_t *entry = NULL;
    caps_filelist_entry_t *flentry = NULL;

    assert(ri != NULL);
    assert(pkg != NULL);
    assert(filepath != NULL);

    if (init_caps(ri)) {
        /* Look for the package in the caps list */
        TAILQ_FOREACH(entry, ri->caps, items) {
            if (!strcmp(entry->pkg, pkg)) {
                break;
            }
        }

        if (entry == NULL) {
            return NULL;
        }

        /* Look for this file's entry for that package */
        TAILQ_FOREACH(flentry, entry->files, items) {
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
