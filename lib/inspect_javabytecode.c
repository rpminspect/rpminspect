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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <ftw.h>
#include <byteswap.h>
#include <assert.h>

#include "rpminspect.h"

/* Globals */
static int prefixlen = 0;
static char *jarfile = NULL;
static short supported_major = -1;
static struct rpminspect *jar_ri = NULL;
static bool jar_result = true;

/*
 * Returns major JVM version found if the file is a compiled Java
 * class file, or -1 if it's not a Java class file.
 */
static short get_jvm_major(const char *filename, const char *localpath,
                           const char *container)
{
    int fd;
    short major;
    char magic[8];

    assert(filename != NULL);
    assert(localpath != NULL);
    assert(container != NULL);

    /* Go ahead and assume Java class filenames end with .class */
    if (strsuffix(filename, CLASS_FILENAME_EXTENSION)) {
        /* read the first 5 bytes and verify it's a Java class */
        fd = open(filename, O_RDONLY | O_CLOEXEC | O_LARGEFILE);

        if (fd == -1) {
            fprintf(stderr, _("unable to open(2) %s from %s for reading: %s\n"), localpath, container, strerror(errno));
            return -1;
        }

        if (read(fd, magic, sizeof(magic)) != sizeof(magic)) {
            fprintf(stderr, _("unable to read(2) %s from %s: %s\n"), localpath, container, strerror(errno));
            return -1;
        }

        if (close(fd) == -1) {
            fprintf(stderr, _("unable to close(2) %s from %s: %s\n"), localpath, container, strerror(errno));
            return -1;
        }

        /* Java class files begin with 0xCAFEBABE */
        if (magic[0] == '\xCA' && magic[1] == '\xFE' && magic[2] == '\xBA' && magic[3] == '\xBE') {
            /* check the major number for compliance */
            memcpy(&major, magic + 6, sizeof(major));

            if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) {
                major = bswap_16(major);
            }

            if (major >= 30) {
                return major;
            }
        }
    }

    return -1;
}

/*
 * Called for each file in the package payload or inside the .jar file.
 */
static bool check_class_file(struct rpminspect *ri, const char *fullpath,
                             const char *localpath, const char *peerfullpath,
                             const char *peerlocalpath, const char *container)
{
    short major, majorpeer;
    struct result_params params;

    assert(fullpath != NULL);
    assert(localpath != NULL);

    init_result_params(&params);
    params.severity = RESULT_BAD;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = HEADER_JAVABYTECODE;

    /* try to see if this is just a .class file */
    major = get_jvm_major(fullpath, localpath, container);

    /* basic checks on the most recent build */
    if (major == -1 && !strsuffix(localpath, CLASS_FILENAME_EXTENSION)) {
        return true;
    } else if (major < 0 || major > 60) {
        xasprintf(&params.msg, _("File %s (%s), Java byte code version %d is incorrect (wrong endianness? corrupted file? space JDK?)"), localpath, container, major);
        add_result(ri, &params);
        free(params.msg);
        return false;
    } else if (major > supported_major) {
        xasprintf(&params.msg, _("File %s (%s), Java byte code version %d greater than supported major version %d for product release %s"), localpath, container, major, supported_major, ri->product_release);
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    /* if a peer exists, perform comparisons on version changes */
    if (peerfullpath && peerlocalpath) {
        majorpeer = get_jvm_major(peerfullpath, peerlocalpath, container);

        if (majorpeer == -1) {
            return true;
        }

        if (major != majorpeer) {
            xasprintf(&params.msg, _("Java byte code version changed from %d to %d in %s from %s"), majorpeer, major, localpath, container);
            add_result(ri, &params);
            free(params.msg);
            return false;
        }
    }

    return true;
}

/*
 * Helper used by nftw() in javabytecode_driver()
 */
static int jar_walker(const char *fpath, __attribute__((unused)) const struct stat *sb, int tflag, __attribute__((unused)) struct FTW *ftwbuf) {
    /* Only looking at regular files */
    if (tflag != FTW_F || !S_ISREG(sb->st_mode)) {
        return 0;
    }

    if (!check_class_file(jar_ri, fpath, fpath+prefixlen, NULL, NULL, jarfile)) {
        jar_result = false;
    }

    return 0;
}

/*
 * Main driver for the inspection.
 */
static bool javabytecode_driver(struct rpminspect *ri, rpmfile_entry_t *file, const char *container)
{
    bool result;
    char *tmppath = NULL;
    int jarstatus = 0;

    if (strsuffix(file->fullpath, JAR_FILENAME_EXTENSION)) {
        /* if we have a possible jar file, try to unpack and walk it */

        /* create a temporary directory to unpack this file */
        xasprintf(&tmppath, "%s/jar.XXXXXX", ri->workdir);

        if ((tmppath = mkdtemp(tmppath)) == NULL) {
            fprintf(stderr, _("*** unable to create a temporary directory for %s: %s\n"), file->fullpath, strerror(errno));
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
        jar_ri = ri;
        jarstatus = nftw(tmppath, jar_walker, FOPEN_MAX, FTW_MOUNT | FTW_PHYS);

        if (jarstatus != 0) {
            /* we errored somewhere, just report it */
            fprintf(stderr, _("*** error walking the unpacked directory tree for %s\n"), file->fullpath);
        }

        /* clean up */
        rmtree(tmppath, true, false);
        free(tmppath);

        result = jar_result;
    } else {
        if (file->peer_file) {
            result = check_class_file(ri, file->fullpath, file->localpath, file->peer_file->fullpath, file->peer_file->localpath, container);
        } else {
            result = check_class_file(ri, file->fullpath, file->localpath, NULL, NULL, container);
        }
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
    string_map_t *hentry = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    /*
     * Get the major JVM version for this product release.
     */
    if (ri->jvm == NULL) {
        warn(_("*** missing JVM version to product release mapping"));
        return false;
    }

    /* look up the JVM major version; fall back on default if not found */
    HASH_FIND_STR(ri->jvm, ri->product_release, hentry);

    if (hentry == NULL) {
        HASH_FIND_STR(ri->jvm, "default", hentry);
    }

    if (hentry == NULL) {
        warn(_("*** missing JVM version to product release mapping"));
        return false;
    }

    supported_major = strtol(hentry->value, NULL, 10);

    if (errno == ERANGE) {
        warn(_("*** invalid JVM major version"));
        return false;
    }

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
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
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
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_JAVABYTECODE;
        add_result(ri, &params);
    }

    return result;
}
