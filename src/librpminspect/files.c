/*
 * Copyright (C) 2019  Red Hat, Inc.
 * Red Hat Author(s):  David Shea <dshea@redhat.com>
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

#include <assert.h>
#include <errno.h>
#include <regex.h>
#include <search.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <rpm/header.h>
#include <rpm/rpmtd.h>

#include <archive.h>
#include <archive_entry.h>

#include "rpminspect.h"

void free_files(rpmfile_t *files)
{
    rpmfile_entry_t *entry;

    if (files == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(files)) {
        entry = TAILQ_FIRST(files);
        TAILQ_REMOVE(files, entry, items);
        headerFree(entry->rpm_header);
        free(entry->fullpath);
        free(entry);
    }

    free(files);
}

/* Extract the RPM, with path "pkg" and extracted header "hdr", to output_dir.
 * Either output_dir or the directory immediately above it must exist.
 */
rpmfile_t * extract_rpm(const char *pkg, Header hdr)
{
    rpmtd td = NULL;
    rpm_count_t td_size;

    const char *rpm_path;
    struct hsearch_data path_table;
    ENTRY e;
    ENTRY *eptr;
    int *rpm_indices = NULL;

    char *output_dir = NULL;
    struct archive *archive = NULL;
    struct archive_entry *entry;
    const char *archive_path;
    mode_t archive_perm;
    int archive_result;

    int i;
    rpmfile_entry_t *file_entry;
    rpmfile_t *file_list = NULL;

    const int archive_flags = ARCHIVE_EXTRACT_SECURE_NODOTDOT | ARCHIVE_EXTRACT_SECURE_SYMLINKS;

    assert(pkg != NULL);
    assert(hdr != NULL);

    /*
     * Create an output directory for the rpm payload.
     * Name the directory the same as the package, but without the ".rpm".
     * If some joker hands us a file that doesn't end in .rpm, slap a ".d" on the end instead.
     */
    if (strsuffix(pkg, ".rpm")) {
        xasprintf(&output_dir, "%.*s", (int) strlen(pkg) - 4, pkg);
    } else {
        xasprintf(&output_dir, "%s.d", pkg);
    }

    if (mkdir(output_dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) {
        fprintf(stderr, "*** Unable to create directory %s: %s\n", output_dir, strerror(errno));
        return NULL;
    }

    /* Payload data and header data is not in the same order. In order to match things up,
     * read all of the filenames from the RPM header into a hash table, with the index into
     * RPM's arrays as the value.
     */
    td = rpmtdNew();
    assert(td != NULL);

    /* Use the RPMTAG_FILENAMES extension tag to uncompress the filenames */
    /* NB: this function returns 1 for success, not RPMRC_OK */
    if (headerGet(hdr, RPMTAG_FILENAMES, td, HEADERGET_MINMEM | HEADERGET_EXT) != 1) {
        fprintf(stderr, "*** Unable to read RPMTAG_FILENAMES from %s\n", pkg);
        goto cleanup;
    }

    /*
     * Allocate the hash table, and allocate an array of ints to store the index data
     * that the hash table entries will point to.
     */
    td_size = rpmtdCount(td);
    if (hcreate_r(td_size, &path_table) == 0) {
        fprintf(stderr, "*** Unable to allocate hash table: %s\n", strerror(errno));
        goto cleanup;
    }

    rpm_indices = calloc(td_size, sizeof(int));
    assert(rpm_indices != NULL);

    for (i = 0; i < (int) td_size; i++) {
        rpm_path = rpmtdNextString(td);

        if (rpm_path == NULL) {
            fprintf(stderr, "*** Error reading RPM metadata for %s\n", pkg);
            goto cleanup;
        }

        rpm_indices[i] = i;
        e.key = (char *) rpm_path;
        e.data = rpm_indices + i;

        if (hsearch_r(e, ENTER, &eptr, &path_table) == 0) {
            fprintf(stderr, "*** Error populating hash table: %s\n", strerror(errno));
            goto cleanup;
        }
    }

    /* Open the file with libarchive */
    archive = archive_read_new();
    assert(archive != NULL);

#if ARCHIVE_VERSION_NUMBER < 3000000
    archive_read_support_compression_all(archive);
#else
    archive_read_support_filter_all(archive);
#endif
    archive_read_support_format_all(archive);

    if (archive_read_open_filename(archive, pkg, 10240) != ARCHIVE_OK) {
        fprintf(stderr, "*** Unable to open %s with libarchive: %s\n", pkg, archive_error_string(archive));
        goto cleanup;
    }

    /* Allocate space for the return value */
    file_list = calloc(1, sizeof(rpmfile_t));
    assert(file_list != NULL);
    TAILQ_INIT(file_list);

    while ((archive_result = archive_read_next_header(archive, &entry)) != ARCHIVE_EOF) {
        if (archive_result == ARCHIVE_RETRY) {
            continue;
        }

        if (archive_result != ARCHIVE_OK) {
            fprintf(stderr, "*** Error reading from archive %s: %s\n", pkg, archive_error_string(archive));
            free_files(file_list);
            file_list = NULL;
            goto cleanup;
        }

        /* Look up this path in the hash table */
        archive_path = archive_entry_pathname(entry);

        if (strprefix(archive_path, "./")) {
            archive_path += 1;
        }

        e.key = (char *) archive_path;
        if (hsearch_r(e, FIND, &eptr, &path_table) == 0) {
            fprintf(stderr, "*** Payload path %s not in RPM metadata\n", archive_path);
            free_files(file_list);
            file_list = NULL;
            goto cleanup;
        }

        /* Create a new rpmfile_entry_t for this file */
        file_entry = calloc(1, sizeof(rpmfile_entry_t));
        assert(file_entry != NULL);

        file_entry->rpm_header = headerLink(hdr);
        memcpy(&file_entry->st, archive_entry_stat(entry), sizeof(struct stat));
        file_entry->idx = *((int *)eptr->data);

        TAILQ_INSERT_TAIL(file_list, file_entry, items);

        /* Are we extracting this file? */
        if (!(S_ISREG(file_entry->st.st_mode) || S_ISDIR(file_entry->st.st_mode) || S_ISLNK(file_entry->st.st_mode))) {
            continue;
        }

        /* Prepend output_dir to the path name */
        xasprintf(&file_entry->fullpath, "%s/%s", output_dir, archive_path);
        archive_entry_set_pathname(entry, file_entry->fullpath);

        /* Ensure the resulting file is user-readable and global-unwritable */
        archive_perm = archive_entry_perm(entry);
        archive_perm |= S_IRUSR;
        archive_perm &= ~S_IWOTH;

        if (S_ISDIR(file_entry->st.st_mode)) {
            archive_perm |= S_IXUSR;
        }

        archive_entry_set_perm(entry, archive_perm);

        /* Write the file to disk */
        if (archive_read_extract(archive, entry, archive_flags) != ARCHIVE_OK) {
            fprintf(stderr, "*** Error extracting %s: %s\n", pkg, archive_error_string(archive));
            free_files(file_list);
            file_list = NULL;
            goto cleanup;
        }
    }

cleanup:
    if (td != NULL) {
        rpmtdFree(td);
    }

    hdestroy_r(&path_table);
    free(rpm_indices);

    if (archive != NULL) {
        archive_read_free(archive);
    }

    free(output_dir);

    return file_list;
}

const char * get_file_path(const rpmfile_entry_t *file)
{
    rpmtd td;
    const char *result = NULL;

    td = rpmtdNew();
    assert(td != NULL);

    if (headerGet(file->rpm_header, RPMTAG_FILENAMES, td, HEADERGET_MINMEM | HEADERGET_EXT) != 1) {
        fprintf(stderr, "*** Unable to read RPMTAG_FILENAMES for %s\n", file->fullpath);
        goto cleanup;
    }

    if (rpmtdSetIndex(td, file->idx) == -1) {
        fprintf(stderr, "*** Invalid file index for %s\n", file->fullpath);
        goto cleanup;
    }

    result = rpmtdGetString(td);

cleanup:
    rpmtdFree(td);

    return result;
}

bool process_file_path(const rpmfile_entry_t *file, regex_t *include_regex, regex_t *exclude_regex)
{
    const char *localpath;

    localpath = get_file_path(file);
    assert(localpath != NULL);

    /* If include is set, the path must match the regex */
    if ((include_regex != NULL) && (regexec(include_regex, localpath, 0, NULL, 0) != 0)) {
        return false;
    }

    /* If exclude is set, the path must not match the regex */
    if ((exclude_regex != NULL) && (regexec(exclude_regex, localpath, 0, NULL, 0) == 0)) {
        return false;
    }

    return true;
}
