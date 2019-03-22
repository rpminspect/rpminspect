/*
 * Copyright (C) 2018-2019  Red Hat, Inc.
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

#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>

#include "rpminspect.h"

/*
 * Determine if a build specification is local or not.
 */
bool is_local_build(const char *build) {
    char cwd[PATH_MAX + 1];
    char *r = NULL;
    struct stat sb;

    if (build == NULL) {
        return false;
    }

    /* Can we read it as a directory off the filesystem? */
    if (access(build, R_OK)) {
        return false;
    }

    if (stat(build, &sb) == -1) {
        fprintf(stderr, "%s (%d): %s\n", __func__, __LINE__, strerror(errno));
        fflush(stderr);
        return false;
    }

    if (!S_ISDIR(sb.st_mode)) {
        return false;
    }

    /*
     * If we got this far, we think it's a build, but do some checking.
     * Let's look for some common log files:
     *     data/logs/src/state.log
     *     data/logs/src/build.log
     *     data/logs/src/root.log
     */
    memset(cwd, '\0', sizeof(cwd));
    r = getcwd(cwd, PATH_MAX);
    assert(r != NULL);

    if (chdir(build) == -1) {
        fprintf(stderr, "%s (%d): %s\n", __func__, __LINE__, strerror(errno));
        fflush(stderr);
        return false;
    }

    if (chdir("data/logs/src") == -1) {
        fprintf(stderr, "%s (%d): %s\n", __func__, __LINE__, strerror(errno));
        fflush(stderr);
        return false;
    }

    if (access("state.log", F_OK|R_OK) || access("build.log", F_OK|R_OK) || access("root.log", F_OK|R_OK)) {
        fprintf(stderr, "*** Unable to find all build logs in data/logs/src (state.log, build.log, root.log) for %s\n", build);
        fflush(stderr);
        return false;
    }

    if (chdir(cwd) == -1) {
        fprintf(stderr, "%s (%d): %s\n", __func__, __LINE__, strerror(errno));
        fflush(stderr);
        return false;
    }

    return true;
}
