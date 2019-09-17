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

/* Command strings used during this inspection */
#define ZCMP_CMD "zcmp"
#define BZCMP_CMD "bzcmp"
#define XZCMP_CMD "xzcmp"
#define ELFCMP_CMD "eu-elfcmp --ignore-build-id --verbose"
#define MSGUNFMT_CMD "msgunfmt"
#define DIFF_CMD "diff"

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
        xasprintf(&tmp, "%s.  Changes to security policy related files require inspection by the Security Response Team.", msg);
    } else {
        tmp = strdup(msg);
    }

    add_result(&ri->results, severity, waiver, HEADER_CHANGEDFILES, tmp, errors, REMEDY_CHANGEDFILES);
    free(tmp);
    return;
}

/*
 * Runs a command and redirects standard out to a temporary file created
 * in this function.  Caller is responsible for removing the temporary
 * file.
 */
static bool run_and_capture(const char *where, char **output, char *cmd,
                            const char *fullpath, char **errors)
{
    int fd;

    assert(where != NULL);
    assert(output != NULL);
    assert(cmd != NULL);
    assert(fullpath != NULL);

    /* Build a temporary file */
    xasprintf(output, "%s/output.XXXXXX", where);

    /* Generate them and then close them */
    fd = mkstemp(*output);

    if (fd == -1) {
        fprintf(stderr, "*** Unable to create temporary file: %s\n", strerror(errno));
        fflush(stderr);
        return false;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "*** Unable to close temporary file: %s\n", strerror(errno));
        fflush(stderr);
        return false;
    }

    /* Run command and capture output */
    return run_cmd(errors, cmd, fullpath, ">", *output, NULL);
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
    char *errors = NULL;
    bool possible_header = false;
    string_entry_t *entry = NULL;
    severity_t severity = RESULT_VERIFY;
    waiverauth_t waiver = WAIVABLE_BY_ANYONE;
    char *before_tmp = NULL;
    char *after_tmp = NULL;
    int fd;
    char magic[4];

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
        goto done;
    }

    /* The architecture is used in reporting messages */
    arch = headerGetString(file->rpm_header, RPMTAG_ARCH);

    /* Get the MIME type of the file, will need that */
    type = get_mime_type(file->fullpath);

    /* Skip Java class files and JAR files (handled elsewhere) */
    if ((!strcmp(type, "application/zip") &&
         strsuffix(file->fullpath, JAR_FILENAME_EXTENSION)) ||
        (!strcmp(type, "application/x-java-applet") &&
         strsuffix(file->fullpath, CLASS_FILENAME_EXTENSION))) {
        goto done;
    }

    /* Skip Python bytecode files (these always change) */
    if (!strcmp(type, "application/octet-stream") &&
        (strsuffix(file->fullpath, PYTHON_PYC_FILE_EXTENSION) ||
         strsuffix(file->fullpath, PYTHON_PYO_FILE_EXTENSION))) {
        /* Double check that this is a Python bytecode file */
        fd = open(file->fullpath, O_RDONLY | O_CLOEXEC | O_LARGEFILE);

        if (fd == -1) {
            fprintf(stderr, "unable to open(2) %s on %s for reading: %s\n", file->localpath, arch, strerror(errno));
            goto done;
        }

        if (read(fd, magic, sizeof(magic)) != sizeof(magic)) {
            fprintf(stderr, "unable to read(2) %s on %s: %s\n", file->localpath, arch, strerror(errno));
            goto done;
        }

        if (close(fd) == -1) {
            fprintf(stderr, "unable to close(2) %s on %s: %s\n", file->localpath, arch, strerror(errno));
            goto done;
        }

        /*
         * Python bytecode files begin with 0x__0D0D0A
         * The __ is a version identifier which changes from time to time
         */
        if (magic[1] == '\x0D' && magic[2] == '\x0D' && magic[3] == '\x0A') {
            goto done;
        }
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
     * Compare compressed files
     *
     * Use the 'cmp' program for each compression format supported to
     * compare the contents.  This will result in a pass even if the
     * compression levels changed between builds.
     */
    if (!strcmp(type, "application/x-gzip")) {
        result = run_cmd(&errors, ZCMP_CMD, file->peer_file->fullpath, file->fullpath, NULL);

        if (result) {
            xasprintf(&msg, "Compressed gzip file %s changed content on %s", file->localpath, arch);
            add_changedfiles_result(ri, msg, errors, severity, waiver);
            result = false;
        }
    } else if (!strcmp(type, "application/x-bzip2")) {
        result = run_cmd(&msg, BZCMP_CMD, file->peer_file->fullpath, file->fullpath, NULL);

        if (result) {
            xasprintf(&errors, "Compressed bzip2 file %s changed content on %s", file->localpath, arch);
            add_changedfiles_result(ri, msg, errors, severity, waiver);
            result = false;
        }
    } else if (!strcmp(type, "application/x-xz")) {
        result = run_cmd(&errors, XZCMP_CMD, file->peer_file->fullpath, file->fullpath, NULL);

        if (result) {
            xasprintf(&msg, "Compressed xz file %s changed content on %s", file->localpath, arch);
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
        result = run_cmd(&errors, ELFCMP_CMD, file->peer_file->fullpath, file->fullpath, NULL);

        if (result) {
            xasprintf(&msg, "ELF file %s changed content on %s", file->localpath, arch);
            add_changedfiles_result(ri, msg, errors, severity, waiver);
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
        if (run_and_capture(ri->workdir, &after_tmp, MSGUNFMT_CMD, file->fullpath, &errors)) {
            xasprintf(&msg, "Error running msgunfmt on %s on %s", file->localpath, arch);
            add_result(&ri->results, RESULT_BAD, NOT_WAIVABLE, HEADER_CHANGEDFILES, msg, errors, REMEDY_CHANGEDFILES);
            result = false;
            goto done;
        }

        if (run_and_capture(ri->workdir, &before_tmp, MSGUNFMT_CMD, file->peer_file->fullpath, &errors)) {
            xasprintf(&msg, "Error running msgunfmt on %s on %s", file->peer_file->localpath, arch);
            add_result(&ri->results, RESULT_BAD, NOT_WAIVABLE, HEADER_CHANGEDFILES, msg, errors, REMEDY_CHANGEDFILES);
            result = false;
            goto done;
        }

        /* Now diff the mo content */
        if (run_cmd(&errors, DIFF_CMD, "-u", before_tmp, after_tmp, NULL)) {
            xasprintf(&msg, "Message catalog %s changed content on %s", file->localpath, arch);
            add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_CHANGEDFILES, msg, errors, REMEDY_CHANGEDFILES);
            result = false;
        }

        /* Remove the temporary files */
        if (unlink(before_tmp) == -1) {
            fprintf(stderr, "*** Unable to remove temporary file %s: %s\n", before_tmp, strerror(errno));
            fflush(stderr);
            result = false;
        }

        if (unlink(after_tmp) == -1) {
            fprintf(stderr, "*** Unable to remove temporary file %s: %s\n", after_tmp, strerror(errno));
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
        if (run_cmd(&errors, DIFF_CMD, "-u", "-w", "--label", file->localpath, file->peer_file->fullpath, file->fullpath, NULL)) {
            xasprintf(&msg, "Public header file %s changed content on %s, Please make sure this does not change the ABI exported by this package.  The output of `diff -uw` follows.", file->localpath, arch);
            add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_CHANGEDFILES, msg, errors, REMEDY_CHANGEDFILES);
            result = false;
        }
    }

    if (!result) {
        goto done;
    }

    /*
     * Finally, anything that gets down to here just compare checksums.
     */
    before_sum = checksum(file->peer_file->fullpath, &file->peer_file->st.st_mode, SHA256SUM);
    after_sum = checksum(file->fullpath, &file->st.st_mode, SHA256SUM);

    if (strcmp(before_sum, after_sum)) {
        xasprintf(&msg, "File %s changed content on %s", file->localpath, arch);
        add_changedfiles_result(ri, msg, errors, severity, waiver);
        result = false;
    }

done:
    free(type);
    free(before_sum);
    free(after_sum);
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
        add_result(&ri->results, RESULT_OK, NOT_WAIVABLE, HEADER_CHANGEDFILES, NULL, NULL, NULL);
    }

    return result;
}
