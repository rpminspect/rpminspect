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
#include <assert.h>

#include "rpminspect.h"

/*
 * Get the basename of the shell from the #! line of a script.
 * Return the name if it's in our list of shells to use.
 * Return if invalid or not found.
 * Caller is responsible for freeing the returned string.
 */
static char *get_shell(const struct rpminspect *ri, const char *fullpath)
{
    char *shell = NULL;
    FILE *fp = NULL;
    char *buf = NULL;
    char *walk = NULL;
    char *start = NULL;
    size_t len;
    int r = 0;
    string_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(ri->shells != NULL);
    assert(fullpath != NULL);

    fp = fopen(fullpath, "r");

    if (fp == NULL) {
        fprintf(stderr, "error opening %s for reading: %s\n", fullpath, strerror(errno));
        fflush(stderr);
        return NULL;
    }

    r = getline(&buf, &len, fp);
    start = buf;

    if (fclose(fp) == -1) {
        fprintf(stderr, "error closing %s: %s\n", fullpath, strerror(errno));
        fflush(stderr);
    }

    if (r == -1) {
        fprintf(stderr, "error reading first line from %s: %s\n", fullpath, strerror(errno));
        fflush(stderr);
    } else if (!strncmp(buf, "#!", 2)) {
        /* trim newlines */
        buf[strcspn(buf, "\n")] = '\0';

        /* shift over to start to the shell (skip #! and spaces) */
        while (*buf != '/' && *buf != ' ' && *buf != '\0') {
            buf++;
        }

        /* get just the executable name (trims options) */
        walk = buf;

        while (*walk != ' ' && *walk != '\0') {
            walk++;
        }

        *walk = '\0';
        walk = basename(buf);

        /* is it a shell we care about? */
        TAILQ_FOREACH(entry, ri->shells, items) {
            if (!strcmp(walk, entry->data)) {
                shell = strdup(walk);
                break;
            }
        }
    }

    free(start);
    return shell;
}

static bool shellsyntax_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    char *type = NULL;
    char *shell = NULL;
    char *before_shell = NULL;
    int exitcode = -1;
    char *before_errors = NULL;
    int before_exitcode = -1;
    char *errors = NULL;
    char *msg = NULL;
    char *tmp = NULL;
    bool extglob = false;

    /* Ignore files in the SRPM */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Get the mime type of the file */
    type = get_mime_type(file);

    if (!strprefix(type, "text/")) {
        return true;
    }

    /* We need the architecture for reporting */
    arch = headerGetString(file->rpm_header, RPMTAG_ARCH);

    /* Get the shell from the #! line */
    shell = get_shell(ri, file->fullpath);

    if (!shell) {
        return true;
    }

    if (file->peer_file) {
        before_shell = get_shell(ri, file->peer_file->fullpath);

        if (!before_shell) {
            xasprintf(&msg, "%s is a shell script but was not before on %s", file->localpath, arch);
        } else if (strcmp(shell, before_shell)) {
            xasprintf(&msg, "%s is a %s script but was a %s script before on %s", file->localpath, shell, before_shell, arch);
        }

        if (msg) {
            add_result(ri, RESULT_INFO, NOT_WAIVABLE, HEADER_SHELLSYNTAX, msg, NULL, REMEDY_SHELLSYNTAX_GAINED_SHELL);
            free(msg);
        }
    }

    /* Run with -n and capture results */
    errors = run_cmd(&exitcode, shell, "-n", file->fullpath, NULL);

    if (before_shell) {
        before_errors = run_cmd(&before_exitcode, before_shell, "-n", file->peer_file->fullpath, NULL);
    }

    /* Special cash for GNU bash, try with extglob */
    if (exitcode && !strcmp(shell, "bash")) {
        free(errors);
        errors = NULL;
        errors = run_cmd(&exitcode, shell, "-n", "-O", "extglob", file->fullpath, NULL);

        if (!exitcode) {
            extglob = true;
            result = false;
        }
    }

    /* Report */
    if (before_shell) {
        if (!before_exitcode && exitcode) {
            xasprintf(&msg, "%s is no longer a valid %s script on %s", file->localpath, shell, arch);
            add_result(ri, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_SHELLSYNTAX, msg, errors, REMEDY_SHELLSYNTAX_BAD);
            free(msg);
            result = false;
        } else if (before_exitcode && !exitcode) {
            xasprintf(&msg, "%s became a valid %s script on %s", file->localpath, shell, arch);

            if (extglob) {
                xasprintf(&tmp, "%s. The script fails with '-n' but passes with '-O extglob'; be sure 'shopt extglob' is set in the script.", msg);
                free(msg);
                msg = tmp;
            }

            add_result(ri, RESULT_INFO, NOT_WAIVABLE, HEADER_SHELLSYNTAX, msg, before_errors, NULL);
            free(msg);
        }
    } else {
        if (!exitcode && extglob) {
            xasprintf(&msg, "%s fails with '-n' but passes with '-O extglob'; be sure 'shopt extglob' is set in the script on %s", file->localpath, arch);
            add_result(ri, RESULT_INFO, NOT_WAIVABLE, HEADER_SHELLSYNTAX, msg, NULL, NULL);
            free(msg);
        } else if (exitcode) {
            xasprintf(&msg, "%s is not a valid %s script on %s", file->localpath, shell, arch);
            add_result(ri, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_SHELLSYNTAX, msg, errors, REMEDY_SHELLSYNTAX_BAD);
            free(msg);
            result = false;
        }
    }

    free(shell);
    free(before_shell);
    return result;
}

/*
 * Main driver for the 'shellsyntax' inspection.
 */
bool inspect_shellsyntax(struct rpminspect *ri) {
    bool result;

    assert(ri != NULL);

    result = foreach_peer_file(ri, shellsyntax_driver);

    if (result) {
        add_result(ri, RESULT_OK, NOT_WAIVABLE, HEADER_SHELLSYNTAX, NULL, NULL, NULL);
    }

    return result;
}
