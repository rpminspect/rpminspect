/*
 * Copyright (C) 2020  Red Hat, Inc.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <err.h>
#include <archive.h>
#include "rpminspect.h"

/*
 * Create a temporary file containing the uncompressed contents of the
 * specified file.  If the file is not compressed, this function just
 * duplicates it over to the temporary file location.  The optional
 * subdir may specify a subdirectory in worksubdir where the temporary
 * file should go (the directory will be created if necessary).
 * Useful for files that may or may not be compressed but if they are
 * we want to decompress them first and look at the uncompressed
 * output for changes.  The caller must both unlink the created output
 * file and free the returned string (which is the path to the created
 * output file).
 */
char *uncompress_file(struct rpminspect *ri, const char *infile, const char *subdir)
{
    int fd = -1;
    struct stat sb;
    char *outfile = NULL;
    char *base = NULL;
    static int mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    int r = -1;
    size_t size = 0;
    void *buf[BUFSIZ];
    struct archive *input = NULL;
    struct archive_entry *entry = NULL;

    assert(ri != NULL);
    assert(ri->workdir != NULL);
    assert(infile != NULL);

    /* subdirectory where the output file goes */
    if (subdir == NULL) {
        outfile = strdup(ri->worksubdir);
    } else {
        xasprintf(&outfile, "%s/%s", ri->worksubdir, subdir);
    }

    assert(outfile != NULL);

    /* see if the output directory exists */
    errno = 0;
    r = stat(outfile, &sb);

    if ((r == -1) && (errno != ENOENT)) {
        warn("stat(%s)", outfile);
        free(outfile);
        return NULL;
    }

    /*
     * only create the subdirectory if we need it (avoids lots of stat
     * calls later)
     */
    if (errno == ENOENT) {
        if (mkdirp(outfile, mode) == -1) {
            warn("mkdirp(%s)", outfile);
            free(outfile);
            return NULL;
        }

        free(outfile);
    }

    /* create the output file that will be uncompressed */
    base = rindex(infile, '/') + 1;
    assert(base != NULL);

    if (subdir == NULL) {
        xasprintf(&outfile, "%s/%s.XXXXXX", ri->worksubdir, base);
    } else {
        xasprintf(&outfile, "%s/%s/%s.XXXXXX", ri->worksubdir, subdir, base);
    }

    assert(outfile != NULL);

    fd = mkstemp(outfile);

    if (fd == -1) {
        warn("mkstemp()");
        free(outfile);
        return NULL;
    }

    /*
     * Read in the input file and uncompress it if necessary.  The
     * uncompressed data is written to the output file we created and
     * that is used for later diff(1) calls.  Use libarchive here so
     * we can handle a wide range of compression formats.
     */
    input = archive_read_new();

    /* initialize only compression filters in libarchive */
#if ARCHIVE_VERSION_NUMBER < 3000000
    archive_read_support_compression_bzip2(input);
    archive_read_support_compression_compress(input);
    archive_read_support_compression_gzip(input);
    archive_read_support_compression_grzip(input);
    archive_read_support_compression_lrzip(input);
#ifdef ARCHIVE_FILTER_LZ4
    archive_read_support_compression_lz4(input);
#endif
    archive_read_support_compression_lzma(input);
    archive_read_support_compression_lzop(input);
    archive_read_support_compression_none(input);
#else
    archive_read_support_filter_bzip2(input);
    archive_read_support_filter_compress(input);
    archive_read_support_filter_gzip(input);
    archive_read_support_filter_grzip(input);
    archive_read_support_filter_lrzip(input);
#ifdef ARCHIVE_FILTER_LZ4
    archive_read_support_filter_lz4(input);
#endif
    archive_read_support_filter_lzma(input);
    archive_read_support_filter_lzop(input);
    archive_read_support_filter_none(input);
#endif

    /*
     * add raw and empty to account for uncompressed files and
     * compressed empty files
     */
    archive_read_support_format_raw(input);
    archive_read_support_format_empty(input);

    /* open the input file, decompress, and write to output */
    r = archive_read_open_filename(input, infile, 16384);

    if (r != ARCHIVE_OK) {
        warn("archive_read_open_filename()");
        close(fd);
        free(outfile);
        return NULL;
    }

    r = archive_read_next_header(input, &entry);

    if (r != ARCHIVE_OK) {
        warn("archive_read_next_header()");
        close(fd);
        free(outfile);
        return NULL;
    }

    while (1) {
        size = archive_read_data(input, buf, BUFSIZ);

        if (size == 0) {
            break;
        }

        if (write(fd, buf, size) == -1) {
            warn("write()");
            close(fd);
            free(outfile);
            return NULL;
        }
    }

    archive_read_free(input);

    /* close up our uncompressed file */
    if (close(fd) == -1) {
        warn("close()");
        free(outfile);
        return NULL;
    }

    return outfile;
}
