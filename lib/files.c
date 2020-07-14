/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
 * Red Hat Author(s):  David Shea <dshea@redhat.com>
 *                     David Cantrell <dcantrell@redhat.com>
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

/**
 * @file files.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @author David Shea &lt;dshea@redhat.com&gt;
 * @date 2019-2020
 * @brief Package extraction and file gathering functions.
 * @copyright GPL-3.0-or-later
 */

#include <assert.h>
#include <errno.h>
#include <regex.h>
#include <search.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/capability.h>

#include <rpm/header.h>
#include <rpm/rpmtd.h>

#include <archive.h>
#include <archive_entry.h>

#include "rpminspect.h"

/**
 * @brief Free rpmfile_t memory.
 *
 * Free the memory allocated for an rpmfile_t list.  Passing NULL to
 * this function has no effect.  The function will free each struct
 * member in each list entry and then free the entire list.
 *
 * @param files Pointer to the rpmfile_t to free.
 */
void free_files(rpmfile_t *files)
{
    rpmfile_entry_t *entry;

    if (files == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(files)) {
        entry = TAILQ_FIRST(files);
        TAILQ_REMOVE(files, entry, items);
        free(entry->fullpath);
        free(entry->localpath);
        free(entry->type);
        free(entry->checksum);
        free(entry);
    }

    free(files);
}

/**
 * @brief Extract the RPM package specified to a working directory.
 *
 * Given a path to an RPM package and its Header, construct an
 * extraction path and extract all of the payload members to that
 * directory.  The function reads the payload member information from
 * the Header and uses libarchive to perform the actual payload
 * extraction.  Returns an rpmfile_t list of all the payload members.
 * The caller is responsible for freeing this returned list.
 *
 * @param pkg Path to the RPM package to extract.
 * @param hdr RPM Header for the specified package.
 * @return rpmfile_t list of all payload members.  The caller is
 *                   responsible for freeing this list.
 */
rpmfile_t *extract_rpm(const char *pkg, Header hdr, char **output_dir)
{
    rpmtd td = NULL;
    rpm_count_t td_size;

    const char *rpm_path;
    struct hsearch_data path_table = { 0 };
    bool path_table_initialized = false;
    ENTRY e;
    ENTRY *eptr;
    int *rpm_indices = NULL;

    char *hardlinkpath = NULL;
    struct archive *archive = NULL;
    struct archive_entry *entry = NULL;
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
    if (strsuffix(pkg, RPM_FILENAME_EXTENSION)) {
        xasprintf(output_dir, "%.*s", (int) strlen(pkg) - 4, pkg);
    } else {
        xasprintf(output_dir, "%s.d", pkg);
    }

    if (mkdir(*output_dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) {
        fprintf(stderr, _("*** Unable to create directory %s: %s\n"), *output_dir, strerror(errno));
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
        /*
         * A failure here indicates an empty payload, which means this
         * package is just storing metadata (e.g., dependencies).
         */
        goto cleanup;
    }

    /*
     * Allocate the hash table, and allocate an array of ints to store the index data
     * that the hash table entries will point to.
     */
    td_size = rpmtdCount(td);

    if (hcreate_r(td_size * 1.25, &path_table) == 0) {
        fprintf(stderr, _("*** Unable to allocate hash table: %s\n"), strerror(errno));
        goto cleanup;
    }

    path_table_initialized = true;

    rpm_indices = calloc(td_size, sizeof(int));
    assert(rpm_indices != NULL);

    for (i = 0; i < (int) td_size; i++) {
        rpm_path = rpmtdNextString(td);

        if (rpm_path == NULL) {
            fprintf(stderr, _("*** Error reading RPM metadata for %s\n"), pkg);
            goto cleanup;
        }

        rpm_indices[i] = i;
        e.key = (char *) rpm_path;
        e.data = rpm_indices + i;

        if (hsearch_r(e, ENTER, &eptr, &path_table) == 0) {
            fprintf(stderr, _("*** Error populating hash table: %s\n"), strerror(errno));
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
        fprintf(stderr, _("*** Unable to open %s with libarchive: %s\n"), pkg, archive_error_string(archive));
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
            fprintf(stderr, _("*** Error reading from archive %s: %s\n"), pkg, archive_error_string(archive));
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
            fprintf(stderr, _("*** Payload path %s not in RPM metadata\n"), archive_path);
            free_files(file_list);
            file_list = NULL;
            goto cleanup;
        }

        /* Create a new rpmfile_entry_t for this file */
        file_entry = calloc(1, sizeof(rpmfile_entry_t));
        assert(file_entry != NULL);

        file_entry->rpm_header = hdr;
        memcpy(&file_entry->st, archive_entry_stat(entry), sizeof(struct stat));
        file_entry->idx = *((int *)eptr->data);

        file_entry->localpath = strdup(archive_path);
        assert(file_entry->localpath);

        file_entry->type = NULL;
        file_entry->checksum = NULL;
        file_entry->cap = NULL;

        TAILQ_INSERT_TAIL(file_list, file_entry, items);

        /* Are we extracting this file? */
        if (!(S_ISREG(file_entry->st.st_mode) || S_ISDIR(file_entry->st.st_mode) || S_ISLNK(file_entry->st.st_mode))) {
            continue;
        }

        /* Prepend output_dir to the path name */
        xasprintf(&file_entry->fullpath, "%s/%s", *output_dir, archive_path);
        archive_entry_set_pathname(entry, file_entry->fullpath);

        /* Ensure the resulting file is user-rw and global-unwritable */
        archive_perm = archive_entry_perm(entry);
        archive_perm |= S_IRUSR | S_IWUSR;
        archive_perm &= ~S_IWOTH;

        if (S_ISDIR(file_entry->st.st_mode)) {
            archive_perm |= S_IXUSR;
        }

        archive_entry_set_perm(entry, archive_perm);

        /* If this is a hard link, update the hardlink destination path */
        if (archive_entry_nlink(entry) > 1) {
            xasprintf(&hardlinkpath, "%s/%s", *output_dir, archive_entry_hardlink(entry));
            archive_entry_set_link(entry, hardlinkpath);
            free(hardlinkpath);
        }

        /* Write the file to disk */
        if (archive_read_extract(archive, entry, archive_flags) != ARCHIVE_OK) {
            fprintf(stderr, _("*** Error extracting %s: %s\n"), pkg, archive_error_string(archive));
            free_files(file_list);
            file_list = NULL;
            goto cleanup;
        }
    }

cleanup:
    if (path_table_initialized) {
        hdestroy_r(&path_table);
    }

    if (archive != NULL) {
        archive_read_free(archive);
    }

    free(rpm_indices);
    rpmtdFree(td);

    return file_list;
}

/**
 * @brief Match specified file to the include or exclude regular expression.
 *
 * Utility function to help determine if an rpmfile_entry_t would be
 * included or excluded per the specified regular expression.  You
 * must pass in either an include_regex or an exclude_regex.  Passing
 * NULL for both causes the function to return true.  Passing both
 * causes it to ignore the exclude_regex and only honor the
 * include_regex.
 *
 * @param file rpmfile_entry_t to match.
 * @param include_regex regex_t to match file paths to include.
 * @param exclude_regex regex_t to match file paths to exclude.
 * @return True if the non-NULL regular expression matched the
 *         rpmfile_entry_t localpath.
 */
bool process_file_path(const rpmfile_entry_t *file, regex_t *include_regex, regex_t *exclude_regex)
{
    /* If include is set, the path must match the regex */
    if ((include_regex != NULL) && (regexec(include_regex, file->localpath, 0, NULL, 0) != 0)) {
        return false;
    }

    /* If exclude is set, the path must not match the regex */
    if ((exclude_regex != NULL) && (regexec(exclude_regex, file->localpath, 0, NULL, 0) == 0)) {
        return false;
    }

    return true;
}

/**
 * @brief Helper for find_file_peers.
 *
 * Returns a hash table keyed by the localpath fields of the file
 * list, with the rpmfile_entry_t items as values.
 *
 * The list cannot be empty.
 *
 * The keys and values use the same pointers as the rpmfile_entry_t
 * and should not be separately freed. The hash table itself must be
 * hdestroy_r'd and freed by the caller.
 *
 * @param list rpmfile_t list to convert to a hash table.
 @ @return Hash table of files in the rpmfile_t list.
 */
static struct hsearch_data *files_to_table(rpmfile_t *list)
{
    struct hsearch_data *table;
    ENTRY e;
    ENTRY *eptr;

    rpmfile_entry_t *iter;

    rpmtd td;
    rpm_count_t td_size;

    /* Use the length of RPMTAG_FILENAMES for the hash table size */
    td = rpmtdNew();
    assert(td != NULL);

    iter = TAILQ_FIRST(list);
    assert(iter);

    if (headerGet(iter->rpm_header, RPMTAG_FILENAMES, td, HEADERGET_MINMEM | HEADERGET_EXT) != 1) {
        fprintf(stderr, _("***Unable to read RPMTAG_FILENAMES\n"));
        return NULL;
    }

    td_size = rpmtdCount(td);
    rpmtdFree(td);

    table = calloc(1, sizeof(*table));
    assert(table);

    if (hcreate_r(td_size * 1.25, table) == 0) {
        fprintf(stderr, _("*** Unable to allocate hash table: %s\n"), strerror(errno));
        free(table);
        return NULL;
    }

    TAILQ_FOREACH(iter, list, items) {
        e.key = iter->localpath;
        e.data = iter;

        if (hsearch_r(e, ENTER, &eptr, table) == 0) {
            fprintf(stderr, _("*** Unable to add %s to hash table: %s\n"), iter->localpath, strerror(errno));
            hdestroy_r(table);
            free(table);
            return NULL;
        }
    }

    return table;
}

/**
 * @brief Helper for find_one_peer
 *
 * @param file rpmfile_entry_t to set peer_file on.
 * @param eptr Hash table entry with the peer_file data.
 */
static void set_peer(rpmfile_entry_t *file, ENTRY *eptr)
{
    rpmfile_entry_t *peer;

    peer = eptr->data;
    eptr->data = NULL;

    file->peer_file = peer;
    peer->peer_file = file;

    return;
}

/**
 * @brief For the given file from "before", attempt to find a matching
 * file in "after".
 *
 * Any time a match is found, the hash table ENTRY's value field will
 * be set to NULL so that the match cannot be used again. For the
 * purposes of adding tests to match peers, this means that attempts
 * must be made in order from best match to worst match. This is an
 * important thing to note. To see if a file from the hash table can
 * be used, we need to check that hsearch_r() returns non-zero *and*
 * the value of eptr->data is not NULL. If an entry is still there but
 * eptr->data is now NULL it means we have already matched it with
 * another peer.
 *
 * In cases where the peer found has changed paths or subpackages,
 * this function will modify the moved_path and moved_subpackage
 * booleans as appropriate.  The purpose of this is to allow
 * inspection functions to both know the probably file peer but also
 * if it moved or not between builds.  This helps with the reporting
 * messages.
 *
 * @param file rpmfile_entry_t with missing peer_file.
 * @param after After build rpmfile_t list.
 * @param after_table Hash table of after build rpmfile_t localpaths.
 */
static void find_one_peer(rpmfile_entry_t *file, rpmfile_t *after, struct hsearch_data *after_table)
{
    ENTRY e;
    ENTRY *eptr = NULL;
    int hsearch_result;
    rpmfile_entry_t *after_file = NULL;
    const char *before_version = NULL;
    const char *after_version = NULL;
    bool has_version;
    const char *before_release = NULL;
    const char *after_release = NULL;
    char *before_vr = NULL;
    char *after_vr = NULL;
    char *search_path = NULL;

    assert(file != NULL);
    assert(after != NULL);
    assert(after_table != NULL);

    /* used in a number of matching checks below */
    after_file = TAILQ_FIRST(after);

    /* Start with the obvious case: the paths match */
    e.key = file->localpath;
    eptr = NULL;
    hsearch_result = hsearch_r(e, FIND, &eptr, after_table);

    if (hsearch_result != 0 && eptr->data != NULL) {
        set_peer(file, eptr);
        return;
    }

    /* Try substituting the version strings */
    before_version = headerGetString(file->rpm_header, RPMTAG_VERSION);
    after_version = headerGetString(after_file->rpm_header, RPMTAG_VERSION);

    /* If the path doesn't have a version in it we can skip these substitutions */
    has_version = (strstr(file->localpath, before_version) != NULL);

    if (has_version && (strcmp(before_version, after_version) != 0)) {
        search_path = strreplace(file->localpath, before_version, after_version);

        e.key = search_path;
        eptr = NULL;
        hsearch_result = hsearch_r(e, FIND, &eptr, after_table);
        free(search_path);

        if (hsearch_result != 0 && eptr->data != NULL) {
            set_peer(file, eptr);
            return;
        }
    }

    /* Try substituting version-release */
    if (has_version) {
        before_release = headerGetString(file->rpm_header, RPMTAG_RELEASE);
        after_release = headerGetString(after_file->rpm_header, RPMTAG_RELEASE);

        xasprintf(&before_vr, "%s-%s", before_version, before_release);
        xasprintf(&after_vr, "%s-%s", after_version, after_release);

        if (strcmp(before_vr, after_vr) != 0) {
            search_path = strreplace(file->localpath, before_vr, after_vr);

            free(before_vr);
            free(after_vr);

            e.key = search_path;
            eptr = NULL;
            hsearch_result = hsearch_r(e, FIND, &eptr, after_table);
            free(search_path);

            if (hsearch_result != 0 && eptr->data != NULL) {
                set_peer(file, eptr);
                return;
            }
        } else {
            free(before_vr);
            free(after_vr);
        }
    }

    /* See if this file peer moved */
    if (file->peer_file == NULL && S_ISREG(file->st.st_mode)) {
        /* .build-id files can be ignored, they always move */
        if (strstr(file->localpath, BUILD_ID_DIR)) {
            return;
        }

        /* look for a possible match for files that move locations */
        TAILQ_FOREACH(after_file, after, items) {
            /* skip files with peers */
            if (after_file->peer_file) {
                continue;
            }

            /* match files that move between subpackages */
            /*
             * This is a best guess that checks the following:
             * - localpath
             * - MIME type
             *
             * This may need refinement down the road to check other things.
             */
            if (strsuffix(after_file->localpath, file->localpath) &&
                !strcmp(get_mime_type(file), get_mime_type(after_file)) &&
                strcmp(headerGetString(file->rpm_header, RPMTAG_NAME), headerGetString(after_file->rpm_header, RPMTAG_NAME))) {
                DEBUG_PRINT("%s probably moved to %s\n", file->localpath, after_file->localpath);

                e.key = after_file->localpath;
                eptr = NULL;
                hsearch_result = hsearch_r(e, FIND, &eptr, after_table);

                if (hsearch_result != 0 && eptr->data != NULL) {
                    set_peer(file, eptr);
                    DEBUG_PRINT("moved subpackage\n");
                    file->moved_subpackage = true;
                    file->peer_file->moved_subpackage = true;
                    return;
                }
            }
        }
    }

    return;
}

/**
 * @brief Find matching files between the before and after lists.
 *
 * Scan the before build and look for matching peer files in the after
 * build.  The peer_file members are populated with each other's
 * entries.  That is, the before build's peer_file will point to the
 * after build file and the after build peer_file will point to the
 * before build file.  If an rpmfile_entry_t peer_file is NULL, it
 * means it has no peer that could be found.
 *
 * @param before Before build package's rpmfile_t list.
 * @param after After build package's rpmfile_t list.
 * @return 0 on success, -1 on failure.
 */
void find_file_peers(rpmfile_t *before, rpmfile_t *after)
{
    struct hsearch_data *after_table = NULL;
    rpmfile_entry_t *before_entry = NULL;

    assert(before != NULL);
    assert(after != NULL);

    /* Make sure there is something to match */
    if (TAILQ_EMPTY(before) || TAILQ_EMPTY(after)) {
        return;
    }

    /* Create a hash table of the after list, mapping path(char *) to rpmfile_entry_t */
    after_table = files_to_table(after);
    assert(after_table);

    /* Match peers */
    TAILQ_FOREACH(before_entry, before, items) {
        find_one_peer(before_entry, after, after_table);
    }

    hdestroy_r(after_table);
    free(after_table);
    return;
}

/**
 * @brief Return the capabilities(7) of the specified rpmfile_entry_t.
 *
 * If the capabilities(7) of the specified file are cached, return
 * that.  Otherwise get and cache the capabilities, then return the
 * cached value.
 *
 * @param file rpmfile_entry_t specifying the file.
 * @return cap_t containing the capabilities(7) of the file.
 */
cap_t get_cap(rpmfile_entry_t *file)
{
    int fd;
    const char *arch = NULL;

    assert(file != NULL);
    arch = get_rpm_header_arch(file->rpm_header);
    assert(arch != NULL);

    if (file->cap) {
        return file->cap;
    }

    assert(file->fullpath != NULL);

    /* Only for regular files */
    if (!S_ISREG(file->st.st_mode)) {
        return NULL;
    }

    /* Gather capabilities(7) for the file we need */
    if ((fd = open(file->fullpath, O_RDONLY)) == -1) {
        fprintf(stderr, _("*** unable to open() %s on %s: %s\n"), file->localpath, arch, strerror(errno));
        return NULL;
    }

    file->cap = cap_get_fd(fd);

    if (close(fd) == -1) {
        fprintf(stderr, _("*** unable to close() %s on %s: %s\n"), file->localpath, arch, strerror(errno));
    }

    return file->cap;
}

/**
 * @brief Determine if a path is a debug or build path.
 *
 * Returns true if the specified path contains any of:
 *     - BUILD_ID_DIR
 *     - DEBUG_PATH
 *     - DEBUG_SRC_PATH
 * A NULL path returns false.
 *
 * @return True if the path contains any of the name substrings, false
 *         otherwise.
 */
bool is_debug_or_build_path(const char *path)
{
    if (path == NULL) {
        return false;
    }

    if (strstr(path, BUILD_ID_DIR) || strstr(path, DEBUG_PATH) || strstr(path, DEBUG_SRC_PATH)) {
        return true;
    }

    return false;
}

/**
 * @brief True if the payload is empty, false otherwise.
 *
 * All we do here is count the payload entries.  If we have more than
 * zero, the payload is not empty.
 *
 * @param filelist List of files in the RPM payload.
 */
bool is_payload_empty(rpmfile_t *filelist) {
    if (filelist == NULL) {
        return true;
    }

    /* Make sure the file list has at least one entry */
    return TAILQ_EMPTY(filelist);
}
