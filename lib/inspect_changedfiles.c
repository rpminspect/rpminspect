/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
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

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <assert.h>

#include "rpminspect.h"

static bool reported = false;

/*
 * Called by changedfiles_driver() to add additional information for
 * files marked as security concerns.
 */
static void add_changedfiles_result(struct rpminspect *ri, struct result_params *params)
{
    assert(ri != NULL);
    assert(params != NULL);

    if (params->waiverauth == WAIVABLE_BY_SECURITY) {
        params->msg = strappend(params->msg, _("  Changes to security policy related files require inspection by the Security Response Team."));
        assert(params->msg != NULL);
    }

    add_result(ri, params);
    reported = true;
    return;
}

/*
 * Runs a command and redirects standard out to a temporary file created
 * in this function.  Caller is responsible for removing the temporary
 * file.
 */
static char *run_and_capture(const char *where, char **output, char *cmd, const char *fullpath, int *exitcode)
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
        warn(_("mkstemp()"));
        return false;
    }

    if (close(fd) == -1) {
        warn(_("close()"));
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
    const char *arch = NULL;
    char *type = NULL;
    char *before_sum = NULL;
    char *after_sum = NULL;
    char *errors = NULL;
    char *short_errors = NULL;
    char *skip_line = NULL;
    int exitcode;
    bool possible_header = false;
    string_entry_t *entry = NULL;
    char *before_tmp = NULL;
    char *after_tmp = NULL;
    char *before_uncompressed_file = NULL;
    char *after_uncompressed_file = NULL;
    char *comptype = NULL;
    int fd;
    char magic[4];
    bool rebase = false;
    struct result_params params;

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

    /*
     * Determine if we are running on a rebased package or just a
     * package update.
     */
    rebase = is_rebase(ri);

    /* The architecture is used in reporting messages */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up the result parameters */
    init_result_params(&params);
    params.severity = rebase ? RESULT_INFO : RESULT_VERIFY;
    params.waiverauth = (params.severity == RESULT_INFO) ? NOT_WAIVABLE : WAIVABLE_BY_ANYONE;
    params.header = HEADER_CHANGEDFILES;
    params.arch = arch;
    params.file = file->localpath;

    /* Set the waiver type if this is a file of security concern */
    if (ri->security_path_prefix) {
        TAILQ_FOREACH(entry, ri->security_path_prefix, items) {
            while (*entry->data != '/') {
                entry->data++;
            }

            if (strprefix(file->localpath, entry->data)) {
                params.severity = RESULT_BAD;
                params.waiverauth = WAIVABLE_BY_SECURITY;
                break;
            }
        }
    }

    /* Get the MIME type of the file, will need that */
    type = get_mime_type(file);

    /* ELF content changing is handled by other inspections */
    if (!strcmp(type, "application/x-pie-executable") || !strcmp(type, "application/x-executable") || !strcmp(type, "application/x-object")) {
        return true;
    }

    /* Skip Java class files and JAR files (handled elsewhere) */
    if ((!strcmp(type, "application/zip") && strsuffix(file->fullpath, JAR_FILENAME_EXTENSION)) ||
        (!strcmp(type, "application/x-java-applet") && strsuffix(file->fullpath, CLASS_FILENAME_EXTENSION))) {
        return true;
    }

    /* Skip Python bytecode files (these always change) */
    if (!strcmp(type, "application/octet-stream") && (strsuffix(file->fullpath, PYTHON_PYC_FILE_EXTENSION) || strsuffix(file->fullpath, PYTHON_PYO_FILE_EXTENSION))) {
        /* Double check that this is a Python bytecode file */
        fd = open(file->fullpath, O_RDONLY | O_CLOEXEC | O_LARGEFILE);

        if (fd == -1) {
            warn(_("open()"));
            return true;
        }

        if (read(fd, magic, sizeof(magic)) != sizeof(magic)) {
            warn(_("read()"));
            return true;
        }

        if (close(fd) == -1) {
            warn(_("close()"));
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
     * Don't assume compressed files are text, so just perform a byte
     * comparison and report if the uncompressed content has changed
     * between builds.  The idea here is that the before and after
     * build could change the compression ratios or other properties
     * but the uncompressed content would be the same.
     */
    if (!strcmp(type, "application/x-gzip") || !strcmp(type, "application/gzip") ||
        !strcmp(type, "application/x-bzip2") || !strcmp(type, "application/bzip2") ||
        !strcmp(type, "application/x-xz") || !strcmp(type, "application/xz")) {
        /* uncompress the files to temporary files for comparison */
        before_uncompressed_file = uncompress_file(ri, file->peer_file->fullpath, HEADER_CHANGEDFILES);
        assert(before_uncompressed_file != NULL);

        after_uncompressed_file = uncompress_file(ri, file->fullpath, HEADER_CHANGEDFILES);
        assert(after_uncompressed_file != NULL);

        /* perform a byte comparison of the uncompressed files */
        exitcode = filecmp(before_uncompressed_file, after_uncompressed_file);

        if (exitcode == -1) {
            /* an error occurred uncompressing the files */
            warnx(_("filecmp(%s, %s)"), before_uncompressed_file, after_uncompressed_file);
        } else if (exitcode == 1) {
            /* the files are different, report */
            if (rindex(type, '/')) {
                comptype = rindex(type, '/') + 1;
                assert(comptype != NULL);
            }

            if (rindex(type, '-')) {
                comptype = rindex(comptype, '-') + 1;
                assert(comptype != NULL);
            }

            xasprintf(&params.msg, _("Compressed %s file %s changed content on %s."), comptype, file->localpath, arch);
            params.verb = VERB_CHANGED;
            params.noun = file->localpath;
            add_changedfiles_result(ri, &params);
        }

        free(before_uncompressed_file);
        free(after_uncompressed_file);

        if (exitcode == 0 || exitcode == 1) {
            goto done;
        }
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
        params.details = run_and_capture(ri->workdir, &after_tmp, MSGUNFMT_CMD, file->fullpath, &exitcode);

        if (exitcode) {
            xasprintf(&params.msg, _("Error running msgunfmt on %s on %s"), file->localpath, arch);
            params.severity = RESULT_BAD;
            params.waiverauth = NOT_WAIVABLE;
            params.remedy = REMEDY_CHANGEDFILES;
            params.verb = VERB_FAILED;
            params.noun = _("msgunfmt on ${FILE}");
            add_result(ri, &params);
            reported = true;
            goto done;
        }

        params.details = run_and_capture(ri->workdir, &before_tmp, MSGUNFMT_CMD, file->peer_file->fullpath, &exitcode);

        if (exitcode) {
            xasprintf(&params.msg, _("Error running msgunfmt on %s on %s"), file->peer_file->localpath, arch);
            params.severity = RESULT_BAD;
            params.waiverauth = NOT_WAIVABLE;
            params.remedy = REMEDY_CHANGEDFILES;
            params.verb = VERB_FAILED;
            params.noun = _("msgunfmt on ${FILE}");
            add_result(ri, &params);
            reported = true;
            goto done;
        }

        /* Now diff the mo content */
        params.details = run_cmd(&exitcode, DIFF_CMD, "-u", before_tmp, after_tmp, NULL);

        /* Remove the temporary files */
        if (unlink(before_tmp) == -1) {
            warn(_("unlink(%s)"), before_tmp);
        }

        if (unlink(after_tmp) == -1) {
            warn(_("unlink(%s)"), after_tmp);
        }

        if (exitcode) {
            xasprintf(&params.msg, _("Message catalog %s changed content on %s"), file->localpath, arch);
            params.severity = rebase ? RESULT_INFO : RESULT_VERIFY;
            params.waiverauth = (params.severity == RESULT_INFO) ? NOT_WAIVABLE : WAIVABLE_BY_ANYONE;
            params.remedy = REMEDY_CHANGEDFILES;
            params.verb = VERB_CHANGED;
            params.noun = _("${FILE}");
            add_result(ri, &params);
            reported = true;
            goto done;
        }
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
        errors = run_cmd(&exitcode, DIFF_CMD, "-u", "-w", "--label", file->localpath, file->peer_file->fullpath, file->fullpath, NULL);

        if (exitcode) {
            /*
             * Skip the diff(1) header since the output from this
             * gives context.
             */
            short_errors = errors;

            if (strlen(short_errors) >= 3) {
                while (strncmp(short_errors, "@@ ", 3)) {
                    skip_line = index(short_errors, '\n');

                    if (skip_line == NULL) {
                        short_errors = errors;
                        break;
                    }

                    short_errors = skip_line + 1;

                    if (*short_errors == '\0') {
                        short_errors = errors;
                        break;
                    }
                }
            }

            if (rebase) {
                xasprintf(&params.msg, _("Public header file %s changed content on %s.  The output of `diff -uw` folloes."), file->localpath, arch);
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
            } else {
                xasprintf(&params.msg, _("Public header file %s changed content on %s.  Please make sure this does not change the ABI exported by this package.  The output of `diff -uw` follows."), file->localpath, arch);
                params.severity = RESULT_VERIFY;
                params.waiverauth = WAIVABLE_BY_ANYONE;
            }

            params.details = short_errors;
            params.verb = VERB_CHANGED;
            params.noun = _("${FILE}");
            add_result(ri, &params);
            reported = true;

            /* details is not allocated here, freeing errors will take care of it */
            params.details = NULL;

            goto done;
        }
    }

    /* Finally, anything that gets down to here just compare checksums. */
    before_sum = checksum(file->peer_file);
    after_sum = checksum(file);

    if (strcmp(before_sum, after_sum)) {
        xasprintf(&params.msg, _("File %s changed content on %s."), file->localpath, arch);
        params.verb = VERB_CHANGED;
        params.noun = _("${FILE}");
        add_changedfiles_result(ri, &params);
    }

done:
    free(params.msg);
    free(params.details);
    free(errors);
    free(before_tmp);
    free(after_tmp);

    if (params.severity >= RESULT_VERIFY && reported) {
        return false;
    } else {
        return true;
    }
}

bool inspect_changedfiles(struct rpminspect *ri)
{
    bool result;
    struct result_params params;

    result = foreach_peer_file(ri, changedfiles_driver, true);

    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_CHANGEDFILES;
        add_result(ri, &params);
    }

    return result;
}
