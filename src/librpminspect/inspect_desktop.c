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

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <ftw.h>

#include "rpminspect.h"

/* Global variables */
static char *file_to_find = NULL;

/*
 * Helper used by nftw() in _validate_desktop_contents()
 */
static int _find_file(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
    /* Only looking at regular files */
    if (tflag != FTW_F) {
        return 0;
    }

    if (strsuffix(fpath, file_to_find)) {
        free(file_to_find);
        file_to_find = strdup(fpath);
        return 1;
    }

    return 0;
}

/*
 * Called by _desktop_driver() to determine if a found file is one we want to
 * look at.  Returns true if it is, false otherwise.
 */
static bool _is_desktop_entry_file(const char *desktop_entry_files_dir, const rpmfile_entry_t *file) {
    assert(desktop_entry_files_dir != NULL);
    assert(file != NULL);

    /* Is this a regular file? */
    if (!file->fullpath || !S_ISREG(file->st.st_mode)) {
        return false;
    }

    /* Make sure we are looking at a desktop file */
    if (!strprefix(file->localpath, desktop_entry_files_dir)) {
        return false;
    }

    if (!strsuffix(file->localpath, ".desktop") && !strsuffix(file->localpath, ".directory")) {
        return false;
    }

    return true;
}

/*
 * Called by _desktop_driver() to run desktop-file-validate on individual desktop
 * files.  The return value is the return value of pclose() which is -1 on failure
 * otherwise it's the return code of desktop-file-validate.  The output of the
 * command (if it exists) is written to result in the calling function.
 */
static int _validate_desktop_file(const char *tool, const char *fullpath, char **result) {
    int ret;
    char *cmd = NULL;
    char *out = NULL;
    char *new = NULL;
    char buf[BUFSIZ];
    FILE *cmdfp = NULL;

    assert(tool != NULL);
    assert(fullpath != NULL);

    /* Run the validation tool */
    xasprintf(&cmd, "%s %s", tool, fullpath);
    cmdfp = popen(cmd, "r");

    if (cmdfp == NULL) {
        fprintf(stderr, "error running %s on %s: %s\n", tool, fullpath, strerror(errno));
        return false;
    }

    /*
     * Read in all of the information back from the command and store
     * it as our result.  Just concatenate the string as we read it
     * back in buffer size chunks.
     */
    while (fgets(buf, sizeof(buf), cmdfp) != NULL) {
        if (*result == NULL) {
            *result = strdup(buf);
        } else {
            xasprintf(&new, "%s%s", *result, buf);
            free(*result);
            *result = new;
        }
    }

    /* Capture the return code from the validation tool */
    ret = pclose(cmdfp);

    if (ret == -1) {
        fprintf(stderr, "error closing %s for %s: %s\n", tool, fullpath, strerror(errno));
        return -1;
    }

    /*
     * Trim the leading filename since those will be reported dby our add_result().
     */
    new = strdup(*result + strlen(fullpath) + 2);

    /*
     * Trim trailing newlines, again for nicer reporting.
     */
    new[strcspn(new, "\n")] = 0;

    free(*result);
    *result = new;
    return ret;
}

/*
 * Validate the Exec= and Icon= lines in a desktop entry file.  False means
 * something didn't validate.  Results are reported from this function.
 */
static bool _validate_desktop_contents(struct rpminspect *ri, const rpmfile_entry_t *file) {
    bool result = true;
    FILE *fp = NULL;
    size_t len;
    char *msg = NULL;
    char *buf = NULL;
    char *tmp = NULL;
    char *walk = NULL;
    char *subtree = NULL;
    char *arch = NULL;
    struct stat sb;

    assert(ri != NULL);
    assert(file != NULL);

    /* Get the package architecture and the extraction subtree */
    arch = headerGetAsString(file->rpm_header, RPMTAG_ARCH);

    tmp = strdup(file->fullpath);
    walk = tmp + (strlen(file->fullpath) - strlen(file->localpath));
    *walk = '\0';
    subtree = strdup(dirname(tmp));
    free(tmp);

    /* Open the desktop entry file */
    fp = fopen(file->fullpath, "r");

    if (fp == NULL) {
        fprintf(stderr, "error opening %s for reading: %s\n", file->fullpath, strerror(errno));
        fflush(stderr);
        return false;
    }

    /*
     * Iterate over the entire file line by line looking for Exec= and Icon=
     * lines.  When found, validate the value after the '='.
     */
    while (getline(&buf, &len, fp) != -1) {
        if (strprefix(buf, "Exec=")) {
            tmp = buf + 5;
            tmp[strcspn(tmp, "\n")] = 0;

            if (*tmp == '/') {
                /* value is absolute, take as-is */
                file_to_find = strdup(tmp);
            } else if (strstr(file->localpath, "/kde4/") && *tmp != '/') {
                /* desktop entry is for KDE4, so look in its directory */
                xasprintf(&file_to_find, "/usr/libexec/kde4/%s", tmp);
            } else {
                /* everything else would be in /usr/bin */
                xasprintf(&file_to_find, "/usr/bin/%s", tmp);
            }

            /*
             * If we get 1 back from nftw(), it means the executable was
             * found and is valid.  If found, the nftw() helper replaces
             * file_to_find with the full path to where it was found.
             */
            if (nftw(subtree, _find_file, 20, FTW_MOUNT|FTW_PHYS) == 1) {
                if (lstat(file_to_find, &sb) == -1) {
                    fprintf(stderr, "error stat'ing %s: %s\n", file_to_find, strerror(errno));
                    fflush(stderr);
                    return false;
                }

                if (!(sb.st_mode & S_IXOTH)) {
                    xasprintf(&msg, "Desktop file %s on %s references executable %s but %s is not executable by all", file->localpath, arch, tmp, tmp);
                    add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, NULL, REMEDY_DESKTOP);
                    free(msg);
                    result = false;
                }
            } else {
                xasprintf(&msg, "Desktop file %s on %s references executable %s but no subpackages contain an executable of that name", file->localpath, arch, tmp);
                add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, NULL, REMEDY_DESKTOP);
                free(msg);
                result = false;
            }

            free(file_to_find);
        } else if (strprefix(buf, "Icon=")) {
            tmp = buf + 5;
            tmp[strcspn(tmp, "\n")] = 0;

            if (*tmp == '/') {
                /* value is absolute, take as-is */
                file_to_find = strdup(tmp);
            } else if (strstr(file->localpath, ".") && *tmp != '/') {
                /* icon file reference, look in directory */
                xasprintf(&file_to_find, "/usr/share/pixmaps/%s", tmp);
            } else {
                /* any other name is a theme icon, skip */
                free(buf);
                buf = NULL;
                continue;
            }

            /*
             * If we get 1 back from nftw(), it means the icon was
             * found and is valid.  If found, the nftw() helper replaces
             * file_to_find with the full path to where it was found.
             */
            if (nftw(subtree, _find_file, 20, FTW_MOUNT|FTW_PHYS) == 1) {
                if (lstat(file_to_find, &sb) == -1) {
                    fprintf(stderr, "error stat'ing %s: %s\n", file_to_find, strerror(errno));
                    fflush(stderr);
                    return false;
                }

                if (!(sb.st_mode & S_IROTH)) {
                    xasprintf(&msg, "Desktop file %s on %s references icon %s but %s is not readable by all", file->localpath, arch, tmp, tmp);
                    add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, NULL, REMEDY_DESKTOP);
                    free(msg);
                    result = false;
                }
            } else {
                xasprintf(&msg, "Desktop file %s on %s references icon %s but no subpackages contain %s", file->localpath, arch, tmp, tmp);
                add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, NULL, REMEDY_DESKTOP);
                free(msg);
                result = false;
            }

            free(file_to_find);
        }

        free(buf);
        buf = NULL;
    }

    /* Close the desktop entry file */
    if (fclose(fp) == -1) {
        fprintf(stderr, "error closing %s: %s\n", file->fullpath, strerror(errno));
        fflush(stderr);
        result = false;
    }

    free(subtree);
    return result;
}

static bool _desktop_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    bool result = true;
    int after_code;
    int before_code;
    char *after_out = NULL;
    char *before_out = NULL;
    char *msg = NULL;
    char *arch = NULL;

    /*
     * Is this a file we should look at?
     * NOTE: Returning 'true' here is like 'continue' in the calling loop.
     */
    if (!_is_desktop_entry_file(ri->desktop_entry_files_dir, file)) {
        return true;
    }

    /* Validate the desktop file */
    after_code = _validate_desktop_file(ri->desktop_file_validate, file->fullpath, &after_out);

    if (file->peer_file && _is_desktop_entry_file(ri->desktop_entry_files_dir, file->peer_file)) {
        /* if we have a before peer, validate the corresponding desktop file */
        before_code = _validate_desktop_file(ri->desktop_file_validate, file->peer_file->fullpath, &before_out);
    }

    if (after_code == -1) {
        result = false;
    }

    /* Report validation results */
    arch = headerGetAsString(file->rpm_header, RPMTAG_ARCH);

    if (ri->before_srpm != NULL && file->peer_file && before_out == NULL && after_out != NULL) {
        xasprintf(&msg, "File %s is no longer a valid desktop entry file on %s; desktop-file-validate reports:", file->localpath, arch);
        add_result(&ri->results, (after_code == 0) ? RESULT_INFO : RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, after_out, REMEDY_DESKTOP);
    } else if (ri->before_srpm != NULL && file->peer_file == NULL && after_out != NULL) {
        xasprintf(&msg, "New file %s is not a valid desktop file on %s; desktop-file-validate reports:", file->localpath, arch);
        add_result(&ri->results, (after_code == 0) ? RESULT_INFO : RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, after_out, REMEDY_DESKTOP);
    } else if (after_out != NULL) {
        xasprintf(&msg, "File %s is not a valid desktop file on %s; desktop-file-validate reports:", file->localpath, arch);
        add_result(&ri->results, (after_code == 0) ? RESULT_INFO : RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, after_out, REMEDY_DESKTOP);
    }

    free(msg);
    free(after_out);
    free(before_out);

    /* Validate the contents of the desktop entry file */
    if (!_validate_desktop_contents(ri, file) && result) {
        result = false;
    }

    return result;
}

/*
 * Main driver for the 'desktop' inspection.
 */
bool inspect_desktop(struct rpminspect *ri) {
    bool result;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    /*
     * The desktop inspection looks at *.desktop and *.directory files
     * under /usr/share/applications and runs desktop-file-validate on
     * them.  The before and after peers are compared for these files.
     * For the after files, the Exec and Icon references are checked.
     */
    result = foreach_peer_file(ri, _desktop_driver);

    if (result) {
        add_result(&ri->results, RESULT_OK, NOT_WAIVABLE, HEADER_DESKTOP, NULL, NULL, NULL);
    }

    return result;
}
