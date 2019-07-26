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

#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ftw.h>

#include "rpminspect.h"

/* Globals */
static int prefixlen = 0;
static char *jarfile = NULL;

/*
 * Returns true if the file is a compiled Java class file.
 */
static bool is_java_class(const char *filename, const char *localpath, const char *container)
{
    int fd;
    char magic[6];

    assert(filename != NULL);
    assert(localpath != NULL);
    assert(container != NULL);

    /* Go ahead and assume Java class filenames end with .class */
    if (strsuffix(filename, ".class")) {
        /* read the first 5 bytes and verify it's a Java class */
        fd = open(filename, O_RDONLY | O_CLOEXEC | O_LARGEFILE);

        if (fd == -1) {
            fprintf(stderr, "unable to open(2) %s from %s for reading: %s\n", localpath, container, strerror(errno));
            return false;
        }

        if (read(fd, magic, sizeof(magic)) != sizeof(magic)) {
            fprintf(stderr, "unable to read(2) %s from %s: %s\n", localpath, container, strerror(errno));
            return false;
        }

        if (close(fd) == -1) {
            fprintf(stderr, "unable to close(2) %s from %s: %s\n", localpath, container, strerror(errno));
            return false;
        }

        /* Java class files begin with 0xCAFEBABE and then a long int >= 30 */
        if (magic[0] == '\xCA' && magic[1] == '\xFE' && magic[2] == '\xBA' && magic[3] == '\xBE') {
            return true;
        }
    }

    return false;
}

/*
 * Helper used by nftw() in _validate_desktop_contents()
 */
static int jar_walker(const char *fpath, __attribute__((unused)) const struct stat *sb, int tflag, __attribute__((unused)) struct FTW *ftwbuf) {
    /* Only looking at regular files */
    if (tflag != FTW_F) {
        return 0;
    }

    /* Only looking at Java .class files */
    if (!is_java_class(fpath, fpath+prefixlen, jarfile)) {
        return 0;
    }

    return 0;
}


/*
 * Main driver for the inspection.
 */
static bool javabytecode_driver(struct rpminspect *ri, rpmfile_entry_t *file, const char *rpmfile)
{
    bool result = true;
    char *tmppath = NULL;
    int jarstatus = 0;

    if (is_java_class(file->fullpath, file->localpath, rpmfile)) {
        printf("|%s| is a Java class file\n", file->fullpath);
        return true;
    }

    /* if we have a possible jar file, try to unpack and walk it */
    if (strsuffix(file->fullpath, ".jar")) {
        /* create a temporary directory to unpack this file */
        xasprintf(&tmppath, "%s/jar.XXXXXX", ri->workdir);

        if ((tmppath = mkdtemp(tmppath)) == NULL) {
            fprintf(stderr, "*** unable to create a temporary directory for %s: %s\n", file->fullpath, strerror(errno));
            return false;
        }

        /* try to unpack this file */
        if (unpack_archive(file->fullpath, tmppath, true)) {
            /* not an archive, just clean up and skip */
            rmtree(tmppath, true, false);
            return true;
        }

        /* iterate over the unpacked jar file */
        prefixlen = strlen(tmppath);
        jarfile = file->localpath;
        jarstatus = nftw(tmppath, jar_walker, 25, FTW_MOUNT | FTW_PHYS);

        if (jarstatus != 0) {
            /* non-zero return means we errored somewhere, just report it */
            fprintf(stderr, "*** error walking the unpacked directory tree for %s\n", file->fullpath);
        }

        /* clean up */
        rmtree(tmppath, true, false);
        free(tmppath);
    }

    return result;
}

/*
 * Main driver for the 'javabytecode' inspection.
 */
bool inspect_javabytecode(struct rpminspect *ri)
{
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    char *container = NULL;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    /*
     * The javabytecode inspection reads Java class files and reports
     * whether or not the minimum byte code version is met as well as
     * the byte code version changing when performing a comparison of
     * two builds.
     *
     * The minimum bytecode version data comes from the configuration
     * file and varies by vendor product release.
     */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are caught by INSPECT_EMPTYRPM */
        if (TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        container = basename(peer->after_rpm);

        TAILQ_FOREACH(file, peer->after_files, items) {
            if (!javabytecode_driver(ri, file, container)) {
                result = false;
            }
        }
    }

    if (result) {
        add_result(&ri->results, RESULT_OK, NOT_WAIVABLE, HEADER_JAVABYTECODE, NULL, NULL, NULL);
    }

    return result;
}
