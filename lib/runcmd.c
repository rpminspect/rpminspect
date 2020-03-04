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

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

/*
 * Generic popen() wrapper to return the output of the process and the
 * exit code (if desired).  This function returns an allocated string of
 * the output from the program that ran or NULL if there was no output.
 *
 * The first argument is a pointer to an int that will hold the exit code
 * from popen().  If this pointer is NULL, then the caller does not want
 * the exit code and the function does nothing.
 *
 * The second argument is the command followed by any additional arguments
 * that should be included with it.  Note that it is not a format string,
 * all of the subsequent arguments need to be strings because they all
 * get concatenated together.
 */
char *run_cmd(int *exitcode, const char *cmd, ...)
{
    va_list ap;
    int status;
    char *output = NULL;
    char *tail = NULL;
    size_t n = BUFSIZ;
    char *buf = NULL;
    FILE *cmdfp = NULL;
    char *built = NULL;
    char *element = NULL;

    assert(cmd != NULL);

    /* Allocate a large buffer to use for building the command */
    built = strdup(cmd);
    assert(built != NULL);

    /* Add the remaining elements */
    va_start(ap, cmd);

    while ((element = va_arg(ap, char *)) != NULL) {
        xasprintf(&output, "%s %s", built, element);
        assert(output != NULL);
        free(built);
        built = output;
    }

    va_end(ap);

    /* always combine stdout and stderr */
    xasprintf(&output, "%s 2>&1", built);
    assert(output != NULL);
    free(built);
    built = output;

    /* shrink memory allocation */
    built = realloc(built, strlen(built) + 1);

    /* run the command */
    cmdfp = popen(built, "r");

    if (cmdfp == NULL) {
        fprintf(stderr, _("error running `%s`: %s\n"), built, strerror(errno));
        fflush(stderr);
        free(built);
        return false;
    }

    /*
     * Read in all of the information back from the command and store
     * it as our result.  Just concatenate the string as we read it
     * back in buffer size chunks.
     */
    output = NULL;
    buf = calloc(1, n);
    assert(buf != NULL);

    while (getline(&buf, &n, cmdfp) != -1) {
        if (output == NULL) {
            output = strdup(buf);
        } else {
            xasprintf(&tail, "%s%s", output, buf);
            assert(tail != NULL);
            free(output);
            output = tail;
        }
    }

    free(buf);

    /* Capture the return code from the validation tool */
    status = pclose(cmdfp);

    if (exitcode != NULL) {
        *exitcode = status;
    }

    if (status == -1) {
        fprintf(stderr, _("error closing `%s`: %s\n"), built, strerror(errno));
        fflush(stderr);
        free(built);
        free(output);
        return NULL;
    }

    /* There may be no results from the tool */
    if (output != NULL) {
        /* Trim trailing newline */
        tail = rindex(output, '\n');

        if (tail != NULL) {
            tail[strcspn(tail, "\n")] = 0;
        }
    }

    free(built);
    return output;
}
