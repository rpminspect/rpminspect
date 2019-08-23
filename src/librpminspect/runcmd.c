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
 * Generic popen() wrapper to return the exit code of the process and the
 * output collected from the command (if any). The output of the command
 * (if it exists) is written to result in the calling function.
 *
 * The first argument is a pointer to a dynamic string that will hold the
 * captured output from the program.
 *
 * The second argument is the command followed by any additional arguments
 * that should be included with it.  Note that it is not a format string,
 * all of the subsequent arguments need to be strings because they all
 * get concatenated together.
 */
int run_cmd(char **result, const char *cmd, ...) {
    va_list ap;
    int ret;
    char *new = NULL;
    char buf[BUFSIZ];
    FILE *cmdfp = NULL;
    char *built = NULL;
    char *element = NULL;

    assert(result != NULL);
    assert(cmd != NULL);

    /* Allocate a large buffer to use for building the command */
    built = strdup(cmd);
    assert(built != NULL);

    /* Add the remaining elements */
    va_start(ap, cmd);

    while ((element = va_arg(ap, char *)) != NULL) {
        xasprintf(&new, "%s %s", built, element);
        assert(new != NULL);
        free(built);
        built = new;
    }

    va_end(ap);

    /* shrink memory allocation */
    built = realloc(built, strlen(built) + 1);

    /* run the command */
    cmdfp = popen(built, "r");

    if (cmdfp == NULL) {
        fprintf(stderr, "error running `%s`: %s\n", built, strerror(errno));
        fflush(stderr);
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
        fprintf(stderr, "error closing `%s`: %s\n", built, strerror(errno));
        fflush(stderr);
        return -1;
    }

    /*
     * There may be no results from the tool
     */
    if (result != NULL && *result != NULL) {
        /*
         * Trim trailing newlines, again for nicer reporting.
         */
        new = strdup(*result);
        new[strcspn(*result, "\n")] = 0;

        free(*result);
        *result = new;
    }

    return ret;
}
