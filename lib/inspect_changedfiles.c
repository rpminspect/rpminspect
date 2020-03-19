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
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

/*
 * Called by changedfiles_driver() to add additional information for
 * files marked as security concerns.
 */
static void add_changedfiles_result(struct rpminspect *ri, const char *msg, char *errors,
                                    const severity_t severity, const waiverauth_t waiver)
{
    char *tmp = NULL;

    assert(ri != NULL);
    assert(msg != NULL);

    if (waiver == WAIVABLE_BY_SECURITY) {
        xasprintf(&tmp, _("%s.  Changes to security policy related files require inspection by the Security Response Team."), msg);
    } else {
        tmp = strdup(msg);
    }

    add_result(ri, severity, waiver, HEADER_CHANGEDFILES, tmp, errors, REMEDY_CHANGEDFILES);
    free(tmp);
    return;
}

/*
 * Runs a command and redirects standard out to a temporary file created
 * in this function.  Caller is responsible for removing the temporary
 * file.
 */
static char *run_and_capture(const char *where, char **output, char *cmd,
                             const char *fullpath, int *exitcode)
{
    int fd;

    assert(where != NULL);
    assert(output != NULL);
    assert(cmd != NULL);
    assert(fullpath != NULL);
    assert(exitcode != NULL);

    /* Build a temporary file */
    xasprintf(output, "%s/output.XXXXXX", where);

    /* Generate them and then close them */
    fd = mkstemp(*output);

    if (fd == -1) {
        fprintf(stderr, _("*** unable to create temporary file: %s\n"), strerror(errno));
        fflush(stderr);
        return false;
    }

    if (close(fd) == -1) {
        fprintf(stderr, _("*** unable to close temporary file: %s\n"), strerror(errno));
        fflush(stderr);
        return false;
    }

    /* Run command and return output */
    return run_cmd(exitcode, cmd, fullpath, ">", *output, NULL);
}

/*
 * Performs all of the tests associated with the changedfiles inspection.
 */
static bool changedfiles_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    char *type = NULL;
    char *before_sum = NULL;
    char *after_sum = NULL;
    char *msg = NULL;
    string_list_t *diffresult = NULL;
    char *difference = NULL;
    char *errors = NULL;
    char *needle = NULL;
    char *part_errors = NULL;
    char *refined_errors = NULL;
    int exitcode;
    bool possible_header = false;
    string_entry_t *entry = NULL;
    severity_t severity = RESULT_VERIFY;
    waiverauth_t waiver = WAIVABLE_BY_ANYONE;
    char *before_tmp = NULL;
    char *after_tmp = NULL;
    int fd;
    char magic[4];
    const char *bv = NULL;
    const char *av = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Skip files without a peer, other inspections handle new/missing files */
    if (!file->peer_file) {
        return true;
    }

    /* Only perform checks on regular files */
    if (!S_ISREG(file->st.st_mode)) {
        return true;
    }

    /* Skip files in the debug path and debug source path */
    if (strprefix(file->localpath, DEBUG_PATH) ||
        strprefix(file->localpath, DEBUG_SRC_PATH)) {
        return true;
    }

    /* Set the waiver type if this is a file of security concern */
    if (ri->security_path_prefix) {
        TAILQ_FOREACH(entry, ri->security_path_prefix, items) {
            while (*entry->data != '/') {
                entry->data++;
            }

            if (strprefix(file->localpath, entry->data)) {
                severity = RESULT_BAD;
                waiver = WAIVABLE_BY_SECURITY;
                break;
            }
        }
    }

    /*
     * Only run this inspection for builds that change versions or
     * the waiver type is WAIVABLE_BY_SECURITY.
     */
    bv = headerGetString(file->peer_file->rpm_header, RPMTAG_VERSION);
    av = headerGetString(file->rpm_header, RPMTAG_VERSION);

    if (!strcmp(bv, av) && waiver != WAIVABLE_BY_SECURITY) {
        return true;
    }

    /* The architecture is used in reporting messages */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Get the MIME type of the file, will need that */
    type = get_mime_type(file);

    /* Skip Java class files and JAR files (handled elsewhere) */
    if ((!strcmp(type, "application/zip") &&
         strsuffix(file->fullpath, JAR_FILENAME_EXTENSION)) ||
        (!strcmp(type, "application/x-java-applet") &&
         strsuffix(file->fullpath, CLASS_FILENAME_EXTENSION))) {
        return true;
    }

    /* Skip Python bytecode files (these always change) */
    if (!strcmp(type, "application/octet-stream") &&
        (strsuffix(file->fullpath, PYTHON_PYC_FILE_EXTENSION) ||
         strsuffix(file->fullpath, PYTHON_PYO_FILE_EXTENSION))) {
        /* Double check that this is a Python bytecode file */
        fd = open(file->fullpath, O_RDONLY | O_CLOEXEC | O_LARGEFILE);

        if (fd == -1) {
            fprintf(stderr, _("unable to open(2) %s on %s for reading: %s\n"), file->localpath, arch, strerror(errno));
            return true;
        }

        if (read(fd, magic, sizeof(magic)) != sizeof(magic)) {
            fprintf(stderr, _("unable to read(2) %s on %s: %s\n"), file->localpath, arch, strerror(errno));
            return true;
        }

        if (close(fd) == -1) {
            fprintf(stderr, _("unable to close(2) %s on %s: %s\n"), file->localpath, arch, strerror(errno));
            return true;
        }

        /*
         * Python bytecode files begin with 0x__0D0D0A
         * The __ is a version identifier which changes from time to time
         */
        if (magic[1] == '\x0D' && magic[2] == '\x0D' && magic[3] == '\x0A') {
            return true;
        }
    }

    /*
     * Compare compressed files
     *
     * Use the 'cmp' program for each compression format supported to
     * compare the contents.  This will result in a pass even if the
     * compression levels changed between builds.
     */
    if (!strcmp(type, "application/x-gzip")) {
        errors = run_cmd(&exitcode, ZCMP_CMD, file->peer_file->fullpath, file->fullpath, NULL);

        if (exitcode) {
            xasprintf(&msg, _("Compressed gzip file %s changed content on %s"), file->localpath, arch);
            add_changedfiles_result(ri, msg, errors, severity, waiver);
            result = false;
        }
    } else if (!strcmp(type, "application/x-bzip2")) {
        errors = run_cmd(&exitcode, BZCMP_CMD, file->peer_file->fullpath, file->fullpath, NULL);

        if (exitcode) {
            xasprintf(&msg, _("Compressed bzip2 file %s changed content on %s"), file->localpath, arch);
            add_changedfiles_result(ri, msg, errors, severity, waiver);
            result = false;
        }
    } else if (!strcmp(type, "application/x-xz")) {
        errors = run_cmd(&exitcode, XZCMP_CMD, file->peer_file->fullpath, file->fullpath, NULL);

        if (exitcode) {
            xasprintf(&msg, _("Compressed xz file %s changed content on %s"), file->localpath, arch);
            add_changedfiles_result(ri, msg, errors, severity, waiver);
            result = false;
        }
    }

    if (!result) {
        goto done;
    }

    /*
     * Compare ELF objects and report any changes.
     */
    if (!strcmp(type, "application/x-pie-executable") ||
        !strcmp(type, "application/x-executable") ||
        !strcmp(type, "application/x-object")) {
        errors = run_cmd(&exitcode, ELFCMP_CMD, file->peer_file->fullpath, file->fullpath, NULL);

        if (exitcode) {
            /*
             * Clean up eu-elfcmp results.  Strike the fullpaths and only show
             * the localpath once.  Keep the eu-elfcmp: prefix.
             */
            xasprintf(&needle, _("%s differ:"), file->localpath);
            part_errors = strstr(errors, needle);

            if (part_errors) {
                xasprintf(&refined_errors, "eu-elfcmp: %s", part_errors);
            } else {
                /* unknown output format from eu-elfcmp */
                refined_errors = strdup(errors);
            }

            xasprintf(&msg, _("ELF file %s changed content on %s"), file->localpath, arch);
            add_changedfiles_result(ri, msg, refined_errors, severity, waiver);
            free(needle);
            free(refined_errors);
            result = false;
        }
    }

    if (!result) {
        goto done;
    }

    /*
     * Compare gettext .mo files and report any changes.
     */
    if (!strcmp(type, "application/x-gettext-translation") &&
        strsuffix(file->localpath, MO_FILENAME_EXTENSION)) {
        /*
         * This one is somewhat complicated.  We run msgunfmt on the mo files,
         * but first we have to make temporary files for that output.  Then
         * invoke diff(1) on those files and capture the output for reporting
         * out.  It's simple, but the fact that we have to use these command
         * line programs makes it a bit complicated.  Patches welcome that do
         * this with a library call or two.
         */

        /* First, unformat the mo files */
        errors = run_and_capture(ri->workdir, &after_tmp, MSGUNFMT_CMD, file->fullpath, &exitcode);

        if (exitcode) {
            xasprintf(&msg, _("Error running msgunfmt on %s on %s"), file->localpath, arch);
            add_result(ri, RESULT_BAD, NOT_WAIVABLE, HEADER_CHANGEDFILES, msg, errors, REMEDY_CHANGEDFILES);
            result = false;
            goto done;
        }

        errors = run_and_capture(ri->workdir, &before_tmp, MSGUNFMT_CMD, file->peer_file->fullpath, &exitcode);

        if (exitcode) {
            xasprintf(&msg, _("Error running msgunfmt on %s on %s"), file->peer_file->localpath, arch);
            add_result(ri, RESULT_BAD, NOT_WAIVABLE, HEADER_CHANGEDFILES, msg, errors, REMEDY_CHANGEDFILES);
            result = false;
            goto done;
        }

        /* Now diff the mo content */
        diffresult = unified_file_diff(before_tmp, after_tmp);

        if (diffresult && !TAILQ_EMPTY(diffresult)) {
            difference = list_to_string(diffresult);
            xasprintf(&msg, _("Message catalog %s changed content on %s"), file->localpath, arch);
            add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_CHANGEDFILES, msg, difference, REMEDY_CHANGEDFILES);
            result = false;
            free(difference);
            list_free(diffresult, free);
        }

        /* Remove the temporary files */
        if (unlink(before_tmp) == -1) {
            fprintf(stderr, _("*** Unable to remove temporary file %s: %s\n"), before_tmp, strerror(errno));
            fflush(stderr);
            result = false;
        }

        if (unlink(after_tmp) == -1) {
            fprintf(stderr, _("*** Unable to remove temporary file %s: %s\n"), after_tmp, strerror(errno));
            fflush(stderr);
            result = false;
        }
    }

    if (!result) {
        goto done;
    }

    /*
     * Compare C and C++ header files and report changes.
     * NOTE:  We check the MIME type of the file and then see if the name
     * ends with .h, .H, .hpp, or .hxx.  The extension list could probably
     * be a configuration file change.  But more importantly, this check
     * excludes any header files that lack a file ending like this.
     */
    if (ri->header_file_extensions) {
        TAILQ_FOREACH(entry, ri->header_file_extensions, items) {
            if (strsuffix(file->localpath, entry->data)) {
                possible_header = true;
                break;
            }
        }
    }

    if (!strcmp(type, "text/x-c") && possible_header) {
        /* Now diff the header content */
        diffresult = unified_file_diff(file->peer_file->fullpath, file->fullpath);

        if (diffresult && !TAILQ_EMPTY(diffresult)) {
            difference = list_to_string(diffresult);
            xasprintf(&msg, _("Public header file %s changed content on %s, Please make sure this does not change the ABI exported by this package.  The output of `diff -uw` follows."), file->localpath, arch);
            add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_CHANGEDFILES, msg, difference, REMEDY_CHANGEDFILES);
            result = false;
            free(difference);
            list_free(diffresult, free);
        }
    }

    if (!result) {
        goto done;
    }

    /* Finally, anything that gets down to here just compare checksums. */
    before_sum = checksum(file->peer_file);
    after_sum = checksum(file);

    if (strcmp(before_sum, after_sum)) {
        xasprintf(&msg, _("File %s changed content on %s"), file->localpath, arch);
        add_changedfiles_result(ri, msg, errors, severity, waiver);
        result = false;
    }

done:
    free(msg);
    free(errors);
    free(before_tmp);
    free(after_tmp);

    return result;
}

bool inspect_changedfiles(struct rpminspect *ri)
{
    bool result;

    result = foreach_peer_file(ri, changedfiles_driver);

    if (result) {
        add_result(ri, RESULT_OK, NOT_WAIVABLE, HEADER_CHANGEDFILES, NULL, NULL, NULL);
    }

    return result;
}
