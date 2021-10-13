/*
 * Copyright © 2019 Red Hat, Inc.
 * Author(s): David Cantrell <dcantrell@redhat.com>
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

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "rpminspect.h"

#define RD STDIN_FILENO
#define WR STDOUT_FILENO

extern char **environ;

/*
 * Generic fork()/execvpe() wrapper to return the output of the
 * process and the exit code (if desired).  This function returns an
 * allocated string of the output from the program that ran or NULL if
 * there was no output.
 *
 * The first argument is a pointer to an int that will hold the exit
 * code from execvpe().  If this pointer is NULL, then the caller does
 * not want the exit code and the function does nothing.
 *
 * The second argument is the command followed by any additional
 * arguments that should be included with it.  Note that it is not a
 * format string, all of the subsequent arguments need to be strings
 * because they all get concatenated together.
 */
char *run_cmd_vpe(int *exitcode, char **argv)
{
    int i = 0;
    int pfd[2];
    int status = 0;
    pid_t proc = 0;
    FILE *reader = 0;
    char *signame = NULL;
    char *output = NULL;
    char *tail = NULL;
    size_t n = BUFSIZ;
    char *buf = NULL;

    assert(argv != NULL);
    assert(argv[0] != NULL);

    /* create pipes to interact with the child */
    if (pipe(pfd) == -1) {
        warn("pipe");
        *exitcode = EXIT_FAILURE;
        return NULL;
    }

    /* run the command */
    proc = fork();

    if (proc == 0) {
        /* connect the output */
        if (dup2(pfd[WR], STDOUT_FILENO) == -1 || dup2(pfd[WR], STDERR_FILENO) == -1) {
            warn("dup2");
            _exit(EXIT_FAILURE);
        }

        /* close the pipe */
        if (close(pfd[RD]) == -1 || close(pfd[WR]) == -1) {
            warn("dup2");
            _exit(EXIT_FAILURE);
        }

        setlinebuf(stdout);

        /* run the command */
        if (execvpe(argv[0], argv, environ) == -1) {
            warn("execvpe");
            _exit(EXIT_FAILURE);
        }
    } else if (proc == -1) {
        /* failure */
        warn("fork");
    } else {
        /* close the pipe */
        if (close(pfd[WR]) == -1) {
            warn("close");
        }

        /*
         * Read in all of the information back from the command and store
         * it as our result.  Just concatenate the string as we read it
         * back in buffer size chunks.
         */
        reader = fdopen(pfd[RD], "r");

        if (reader == NULL) {
            *exitcode = EXIT_FAILURE;
            return NULL;
        }

        output = NULL;
        buf = calloc(1, n);
        assert(buf != NULL);

        while (getline(&buf, &n, reader) != -1) {
            xasprintf(&tail, "%s%s", (output == NULL) ? "" : output, buf);
            assert(tail != NULL);
            free(output);
            output = tail;
        }

        free(buf);

        if (fclose(reader) == -1) {
            warn("fclose");
        }

        /* wait for the command */
        if (waitpid(proc, &status, 0) == -1) {
            warn("waitpid");
            *exitcode = EXIT_FAILURE;
        }

        if (WIFEXITED(status)) {
            *exitcode = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            *exitcode = EXIT_FAILURE;

            /* generate a string with the signal name if possible */
            i = WTERMSIG(status);

            if (strsignal(i) == NULL) {
                xasprintf(&signame, _("%d"), i);
            } else {
                xasprintf(&signame, _("%d (%s)"), i, strsignal(i));
            }

            /* generic output indicating the command we tried to run and the signal received */
            if (output) {
                xasprintf(&tail, _("%s\n\n%s tried to run the command and it received signal %s"), output, COMMAND_NAME, signame);
                free(output);
                output = tail;
            } else {
                xasprintf(&output, _("%s tried to run the command and it received signal %s"), COMMAND_NAME, signame);
            }

            free(signame);
        }

        /* There may be no results from the tool */
        if (output != NULL) {
            /* Trim trailing newline */
            tail = rindex(output, '\n');

            if (tail != NULL) {
                tail[strcspn(tail, "\n")] = 0;
            }
        }
    }

    return output;
}

/*
 * Wrapper for run_cmd_vpe() that lets you pass in varargs instead of a
 * string_list_t.
 */
char *run_cmd(int *exitcode, const char *cmd, ...)
{
    va_list ap;
    char *output = NULL;
    char *element = NULL;
    char **argv = NULL;
    int i = 0;

    assert(cmd != NULL);

    /* convert varargs to a string array */
    argv = calloc(2, sizeof(*argv));
    assert(argv != NULL);

    /* add the command */
    argv[i] = strdup(cmd);
    assert(argv[i] != NULL);
    i++;
    argv[i] = NULL;

    /* Add the remaining elements */
    va_start(ap, cmd);

    while ((element = va_arg(ap, char *)) != NULL) {
        i++;
        argv = reallocarray(argv, i + 1, sizeof(*argv));
        assert(argv != NULL);

        argv[i - 1] = strdup(element);
        assert(argv[i - 1] != NULL);
        argv[i] = NULL;
    }

    va_end(ap);

    /* run the command */
    output = run_cmd_vpe(exitcode, argv);

    /* clean up */
    free_argv(argv);

    return output;
}

/*
 * Split a string in to a char ** of all the arguments, terminated
 * with a NULL entry.  Caller must free.
 */
char **build_argv(const char *cmd)
{
    char **r = NULL;
    int i = 0;
    string_list_t *list = NULL;
    string_entry_t *entry = NULL;

    if (cmd == NULL) {
        return NULL;
    }

    list = strsplit(cmd, " \t");
    assert(list != NULL);

    TAILQ_FOREACH(entry, list, items) {
        r = reallocarray(r, i + 2, sizeof(*r));
        assert(r != NULL);

        r[i] = strdup(entry->data);
        assert(r[i] != NULL);
        r[i + 1] = NULL;
        i++;
    }

    list_free(list, free);
    return r;
}

/*
 * Clean up a char **
 */
void free_argv(char **argv)
{
    int i = 0;

    if (argv == NULL) {
        return;
    }

    for (i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }

    free(argv);
    return;
}

/*
 * Free one of the command line option tables.
 */
void free_argv_table(struct rpminspect *ri, string_list_map_t *table)
{
    string_list_map_t *hentry = NULL;
    string_list_map_t *tmp_hentry = NULL;

    assert(ri != NULL);
    assert(ri->arches != NULL);

    if (table == NULL) {
        return;
    }

    HASH_ITER(hh, table, hentry, tmp_hentry) {
        HASH_DEL(table, hentry);
        free(hentry->key);
        list_free(hentry->value, free);
    }

    return;
}
