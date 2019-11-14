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
#include <libgen.h>
#include <ftw.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>

#include "rpminspect.h"

/* Global variables */
static char *file_to_find = NULL;

/*
 * Helper used by nftw() in validate_desktop_contents()
 */
static int find_file(const char *fpath, __attribute__((unused)) const struct stat *sb,
                     int tflag, __attribute__((unused)) struct FTW *ftwbuf)
{
    char *tmpbuf = NULL;
    char *last = NULL;

    /* Only looking at regular files */
    /*
     * XXX: should FTW_SLN have special handling here?  does packaging policy
     * allow packages to ship absolute symlinks to paths not provided by the
     * package itself?
     */
    if (tflag == FTW_D || tflag == FTW_DNR || tflag == FTW_DP || tflag == FTW_NS) {
        return 0;
    }

    /* Look for this name as the basename */
    if (strsuffix(fpath, file_to_find)) {
        free(file_to_find);
        file_to_find = strdup(fpath);
        return 1;
    }

    /* Might be a base name missing a graphics format ending */
    /*
     * This will take an fpath like:
     *     iconfile.png
     *     iconfile.jpg
     *     org.Organization.IconFile
     * And will trim the end of the string from the last period
     * to the end.  So if a desktop entry file specifies 'iconfile'
     * and the package provides iconfile.* somewhere as a file, this
     * will pass.
     */
    tmpbuf = strdup(fpath);
    last = basename(tmpbuf);

    if (last != NULL) {
        if (strprefix(last, file_to_find)) {
            free(tmpbuf);
            free(file_to_find);
            file_to_find = strdup(fpath);
            return 1;
        }
    }

    free(tmpbuf);
    return 0;
}

/*
 * Called by desktop_driver() to determine if a found file is one we want to
 * look at.  Returns true if it is, false otherwise.
 */
static bool is_desktop_entry_file(const char *desktop_entry_files_dir,
                                  const rpmfile_entry_t *file)
{
    assert(desktop_entry_files_dir != NULL);
    assert(file != NULL);

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return false;
    }

    /* Is this a regular file? */
    if (!file->fullpath || !S_ISREG(file->st.st_mode)) {
        return false;
    }

    /* Make sure we are looking at a desktop file */
    if (!strprefix(file->localpath, desktop_entry_files_dir)) {
        return false;
    }

    if (!strsuffix(file->localpath, DESKTOP_FILENAME_EXTENSION) &&
        !strsuffix(file->localpath, DIRECTORY_FILENAME_EXTENSION)) {
        return false;
    }

    return true;
}

/*
 * Validate the Exec= and Icon= lines in a desktop entry file.  False means
 * something didn't validate.  Results are reported from this function.
 */
static bool validate_desktop_contents(struct rpminspect *ri, const rpmfile_entry_t *file)
{
    bool result = true;
    FILE *fp = NULL;
    size_t len;
    char *msg = NULL;
    char *buf = NULL;
    char *tmp = NULL;
    char *walk = NULL;
    char *allpkgtrees = NULL;
    const char *arch = NULL;
    char *exectoken = NULL;
    char *iconpath = NULL;
    struct stat sb;
    bool found = false;
    string_entry_t *entry = NULL;
    DIR *dfd = NULL;
    struct dirent *de = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* Get the package architecture and the extraction subtree */
    arch = headerGetString(file->rpm_header, RPMTAG_ARCH);

    tmp = strdup(file->fullpath);
    walk = tmp + (strlen(file->fullpath) - strlen(file->localpath));
    *walk = '\0';
    allpkgtrees = strdup(dirname(tmp));
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
            /* Take everything after the key and trim newlines */
            tmp = buf + 5;
            tmp[strcspn(tmp, "\n")] = 0;

            /* The Exec line may specify arguments to the program, strip those */
            exectoken = index(tmp, ' ');

            if (exectoken != NULL) {
                *exectoken = '\0';
            }

            /* Figure out how to look for the file */
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
            if (nftw(allpkgtrees, find_file, 25, FTW_MOUNT|FTW_PHYS) == 1) {
                if (lstat(file_to_find, &sb) == -1) {
                    fprintf(stderr, "error stat'ing %s: %s\n", file_to_find, strerror(errno));
                    fflush(stderr);
                    free(buf);
                    return false;
                }

                if (!(sb.st_mode & S_IXOTH)) {
                    xasprintf(&msg, "Desktop file %s on %s references executable %s but %s is not executable by all", file->localpath, arch, tmp, tmp);
                    add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, NULL, REMEDY_DESKTOP);
                    free(msg);
                    result = false;
                }
            } else {
                xasprintf(&msg, "Desktop file %s on %s references executable %s but no subpackages contain an executable of that name", file->localpath, arch, tmp);
                add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, NULL, REMEDY_DESKTOP);
                free(msg);
                result = false;
            }

            free(file_to_find);
        } else if (strprefix(buf, "Icon=")) {
            tmp = buf + 5;
            tmp[strcspn(tmp, "\n")] = 0;
            file_to_find = strdup(tmp);
            found = false;

            /*
             * For each desktop icon path, check each extracted subpackage
             * for the icon.
             */
            TAILQ_FOREACH(entry, ri->desktop_icon_paths, items) {
                dfd = opendir(allpkgtrees);
                assert(dfd != NULL);

                while ((de = readdir(dfd)) != NULL) {
                    if (de->d_type != DT_DIR) {
                        continue;
                    }

                    xasprintf(&iconpath, "%s/%s/%s", allpkgtrees, de->d_name, entry->data);

                    /*
                     * If we get 1 back from nftw(), it means the icon was
                     * found and is valid.  If found, the nftw() helper replaces
                     * file_to_find with the full path to where it was found.
                     */
                    if (nftw(iconpath, find_file, 25, FTW_MOUNT|FTW_PHYS) == 1) {
                        if (lstat(file_to_find, &sb) == -1) {
                            free(iconpath);
                            continue;
                        }

                        found = true;

                        if (!(sb.st_mode & S_IROTH)) {
                            xasprintf(&msg, "Desktop file %s on %s references icon %s but %s is not readable by all", file->localpath, arch, tmp, tmp);
                            add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, NULL, REMEDY_DESKTOP);
                            free(msg);
                            result = false;
                        }
                    }

                    free(iconpath);

                    if (found) {
                        break;
                    }
                }

                if (closedir(dfd) == -1) {
                    fprintf(stderr, "*** error closing directory: %s\n", strerror(errno));
                    fflush(stderr);
                }

                if (found) {
                    break;
                }
            }

            if (!found) {
                xasprintf(&msg, "Desktop file %s on %s references icon %s but no subpackages contain %s", file->localpath, arch, tmp, tmp);
                add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, NULL, REMEDY_DESKTOP);
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

    free(allpkgtrees);
    return result;
}

static bool desktop_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    int after_code;
    char *after_out = NULL;
    char *before_out = NULL;
    char *msg = NULL;
    const char *arch = NULL;
    char *tmpbuf = NULL;

    /*
     * Is this a file we should look at?
     * NOTE: Returning 'true' here is like 'continue' in the calling loop.
     */
    if (!is_desktop_entry_file(ri->desktop_entry_files_dir, file)) {
        return true;
    }

    /* Validate the desktop file */
    after_code = run_cmd(&after_out, DESKTOP_FILE_VALIDATE_CMD, file->fullpath, NULL);
    tmpbuf = strreplace(after_out, file->fullpath, file->localpath);
    free(after_out);
    after_out = tmpbuf;

    if (file->peer_file && is_desktop_entry_file(ri->desktop_entry_files_dir, file->peer_file)) {
        /* if we have a before peer, validate the corresponding desktop file */
        (void) run_cmd(&before_out, DESKTOP_FILE_VALIDATE_CMD, file->peer_file->fullpath, NULL);
        tmpbuf = strreplace(before_out, file->peer_file->fullpath, file->peer_file->localpath);
        free(before_out);
        before_out = tmpbuf;
    }

    if (after_code == -1) {
        result = false;
    }

    /* Report validation results */
    arch = headerGetString(file->rpm_header, RPMTAG_ARCH);

    if (file->peer_file && before_out == NULL && after_out != NULL) {
        xasprintf(&msg, "File %s is no longer a valid desktop entry file on %s; desktop-file-validate reports:", file->localpath, arch);
        add_result(ri, (after_code == 0) ? RESULT_INFO : RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, after_out, REMEDY_DESKTOP);
    } else if (file->peer_file == NULL && after_out != NULL) {
        xasprintf(&msg, "New file %s is not a valid desktop file on %s; desktop-file-validate reports:", file->localpath, arch);
        add_result(ri, (after_code == 0) ? RESULT_INFO : RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, after_out, REMEDY_DESKTOP);
    } else if (after_out != NULL) {
        xasprintf(&msg, "File %s is not a valid desktop file on %s; desktop-file-validate reports:", file->localpath, arch);
        add_result(ri, (after_code == 0) ? RESULT_INFO : RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_DESKTOP, msg, after_out, REMEDY_DESKTOP);
    }

    free(msg);
    free(after_out);
    free(before_out);

    /* Validate the contents of the desktop entry file */
    if (!validate_desktop_contents(ri, file) && result) {
        result = false;
    }

    return result;
}

/*
 * Main driver for the 'desktop' inspection.
 */
bool inspect_desktop(struct rpminspect *ri)
{
    bool result;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    /*
     * The desktop inspection looks at *.desktop and *.directory files
     * under /usr/share/applications and runs desktop-file-validate on
     * them.  The before and after peers are compared for these files.
     * For the after files, the Exec and Icon references are checked.
     */
    result = foreach_peer_file(ri, desktop_driver);

    if (result) {
        add_result(ri, RESULT_OK, NOT_WAIVABLE, HEADER_DESKTOP, NULL, NULL, NULL);
    }

    return result;
}
