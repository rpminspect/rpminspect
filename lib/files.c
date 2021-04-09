/*
 * Copyright © 2019 Red Hat, Inc.
 * Author(s): David Shea <dshea@redhat.com>
 *            David Cantrell <dcantrell@redhat.com>
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

/**
 * @file files.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @author David Shea &lt;dshea@redhat.com&gt;
 * @date 2019-2021
 * @brief Package extraction and file gathering functions.
 * @copyright LGPL-3.0-or-later
 */

#include <assert.h>
#include <errno.h>
#include <err.h>
#include <regex.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/capability.h>

#include <rpm/header.h>
#include <rpm/rpmtd.h>
#include <rpm/rpmfi.h>

#include <archive.h>
#include <archive_entry.h>

#include "rpminspect.h"
#include "uthash.h"

/*
 * hash table used for file entries
 * Not all values are used each time this hash table is implemeneted.
 * See below.  One place uses the path and index and another place
 * uses the path and rpmfile.
 */
struct file_data {
    char *path;                  /* key, the localpath of the file */
    int index;                   /* RPM header index of this file */
    rpmfile_entry_t *rpmfile;    /* rpmfile_entry_t for this file */
    UT_hash_handle hh;           /* makes this structure hashable */
};

/**
 * @brief Given an RPM Header and index, return the RPMTAG_FILEFLAGS
 * entry.
 *
 * @param h RPM Header
 * @param i Index of the RPMTAG_FILEFLAGS entry
 * @return rpmfileAttrs value for the given file index
 */
static rpmfileAttrs get_rpmtag_fileflags(const Header h, const int i)
{
    rpmfileAttrs flags = 0;
    struct rpmtd_s td;
    struct rpmtd_s filenames;
    int idx = -1;

    assert(h != NULL);
    assert(i >= 0);

    if (headerGet(h, RPMTAG_BASENAMES, &filenames, HEADERGET_MINMEM)) {
        if (headerGet(h, RPMTAG_FILEFLAGS, &td, HEADERGET_MINMEM)) {
            idx = rpmtdSetIndex(&td, i);
            assert(idx != -1);
            flags = *(rpmtdGetUint32(&td));
            rpmtdFreeData(&td);
        }

        rpmtdFreeData(&filenames);
    }

    return flags;
}

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

    const char *rpm_path;
    struct file_data *path_table = NULL;
    struct file_data *path_entry = NULL;
    struct file_data *tmp_entry = NULL;

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
        warn("mkdir()");
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
     * Populate the hash table, and allocate an array of ints to store the index data
     * that the hash table entries will point to.
     */
    for (i = 0; i < (int) rpmtdCount(td); i++) {
        rpm_path = rpmtdNextString(td);

        if (rpm_path == NULL) {
            warn("rpmtdNextString()");
            goto cleanup;
        }

        path_entry = calloc(1, sizeof(*path_entry));
        assert(path_entry != NULL);
        path_entry->path = strdup(rpm_path);
        path_entry->index = i;
        HASH_ADD_KEYPTR(hh, path_table, path_entry->path, strlen(path_entry->path), path_entry);
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
        warn("archive_read_open_filename()");
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
            warn("archive_read_next_header()");
            free_files(file_list);
            file_list = NULL;
            goto cleanup;
        }

        /* Look up this path in the hash table */
        archive_path = archive_entry_pathname(entry);

        if (strprefix(archive_path, "./")) {
            archive_path += 1;
        }

        HASH_FIND_STR(path_table, archive_path, path_entry);

        if (path_entry == NULL) {
            warn(_("*** payload path %s not in RPM metadata"), archive_path);
            free_files(file_list);
            file_list = NULL;
            goto cleanup;
        }

        /* Create a new rpmfile_entry_t for this file */
        file_entry = calloc(1, sizeof(rpmfile_entry_t));
        assert(file_entry != NULL);

        file_entry->rpm_header = hdr;
        memcpy(&file_entry->st, archive_entry_stat(entry), sizeof(struct stat));
        file_entry->idx = path_entry->index;

        file_entry->localpath = strdup(archive_path);
        assert(file_entry->localpath);

        file_entry->flags = get_rpmtag_fileflags(hdr, file_entry->idx);
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
            warn("archive_read_extract()");
            free_files(file_list);
            file_list = NULL;
            goto cleanup;
        }
    }

cleanup:
    HASH_ITER(hh, path_table, path_entry, tmp_entry) {
        HASH_DEL(path_table, path_entry);
        free(path_entry->path);
        free(path_entry);
    }

    if (archive != NULL) {
        archive_read_free(archive);
    }

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
 * destroyed and freed by the caller.  NOTE: The path value in the
 * returned hash table is not malloc'ed.  Only each hash table entry
 * is malloc'ed, so take care free'ing.
 *
 * @param list rpmfile_t list to convert to a hash table.
 * @return Hash table of files in the rpmfile_t list.
 */
static struct file_data *files_to_table(rpmfile_t *list)
{
    struct file_data *table = NULL;
    struct file_data *fentry = NULL;
    rpmfile_entry_t *iter = NULL;

    assert(list != NULL);
    assert(!TAILQ_EMPTY(list));

    iter = TAILQ_FIRST(list);
    assert(iter);

    TAILQ_FOREACH(iter, list, items) {
        fentry = calloc(1, sizeof(*fentry));
        assert(fentry != NULL);
        fentry->path = iter->localpath;
        fentry->rpmfile = iter;
        HASH_ADD_KEYPTR(hh, table, fentry->path, strlen(fentry->path), fentry);
    }

    return table;
}

/**
 * @brief Helper for find_one_peer
 *
 * @param file rpmfile_entry_t to set peer_file on.
 * @param entry Hash table entry with the peer_file data.
 */
static void set_peer(rpmfile_entry_t *file, struct file_data *fentry)
{
    rpmfile_entry_t *peer = NULL;

    peer = fentry->rpmfile;
    fentry->rpmfile = NULL;

    file->peer_file = peer;
    peer->peer_file = file;

    return;
}

/**
 * @brief Helper for find_one_peer.  Turns version numbers embedded in
 * certain filenames to generic placeholders.  For example, it would
 * make these changes:
 *
 *     /usr/lib/libNAME.so.1.2.3 -> /usr/lib/libNAME.so.?.?.?
 *     /usr/lib/debug/usr/lib/libNAME.so.1.2.3-1.47.2-5.x86_64.debug -> /usr/lib/debug/usr/lib/libNAME.so.?.?.?-?.?.?-?.x86_64.debug
 *
 * The purpose of these changes is to make finding file peers easier
 * between different versions of packages.
 *
 * @param s The string containing version substrings to convert.
 * @param ignore Optional string specifying a token string to ignore.
 * @return The newly created string with generic version number
 * substrings.  This string must be freed by the caller.
 */
static char *comparable_version_substrings(const char *s, const char *ignore)
{
    char *orig = NULL;
    char *inner_orig = NULL;
    char *outer_orig = NULL;
    int reg_result = 0;
    char *outer_token = NULL;
    char *inner_token = NULL;
    regex_t num_regex;
    char reg_error[BUFSIZ];
    char *result = NULL;
    int ignore_result = false;
    size_t i = 0;
    bool first = true;
    bool same = true;

    assert(s != NULL);

    /* match number segments of the tail using a regex */
    reg_result = regcomp(&num_regex, "^[0-9_-]+$", REG_EXTENDED);

    if (reg_result != 0) {
        regerror(reg_result, &num_regex, reg_error, sizeof(reg_error));
        warn("regcomp(): %s", reg_error);
        return NULL;
    }

    /* make a copy of the input */
    orig = outer_orig = strdup(s);

    /* iterate over the string tokens */
    while ((outer_token = strsep(&outer_orig, "/")) != NULL) {
        if (!strcmp(outer_token, "")) {
            continue;
        }

        first = true;

        if (result == NULL) {
            result = strdup("/");
        } else {
            result = strappend(result, "/", NULL);
        }

        /* the outer tokens are directory parts, see if there's a versioned one */
        if (!regexec(&num_regex, outer_token, 0, NULL, 0) || strcmp(outer_token, "lib64")) {
            inner_orig = strdup(outer_token);

            /* there is, break down this token in to version number parts */
            while ((inner_token = strsep(&outer_token, ".")) != NULL) {
                /* if the caller provided an ignore string, check it */
                if (ignore == NULL) {
                    ignore_result = 0;
                } else {
                    ignore_result = strcmp(inner_token, ignore);
                }

                /* add back the version number delimiters */
                if (first) {
                    first = false;
                } else {
                    result = strappend(result, ".", NULL);
                }

                /* make the version substring generic */
                same = true;

                if (!regexec(&num_regex, inner_token, 0, NULL, 0) || (strcmp(inner_token, DEBUG_SUBSTRING) && ignore_result)) {
                    for (i = 0; i < strlen(inner_token); i++) {
                        if (isdigit(inner_token[i])) {
                            inner_token[i] = '?';
                        } else {
                            same = false;
                        }
                    }
                } else {
                    same = false;
                }

                /* append the inner token */
                if (same) {
                    result = strappend(result, "?", NULL);
                } else {
                    result = strappend(result, inner_token, NULL);
                }
            }

            free(inner_orig);
        } else {
            /* nothing special, just append the outer token */
            result = strappend(result, outer_token, NULL);
        }
    }

    /* clean up */
    free(orig);
    regfree(&num_regex);

    return result;
}

/**
 * @brief For the given file from "before", attempt to find a matching
 * file in "after".
 *
 * Any time a match is found, the hash table entry's value field will
 * be set to NULL so that the match cannot be used again. For the
 * purposes of adding tests to match peers, this means that attempts
 * must be made in order from best match to worst match. This is an
 * important thing to note.  If an entry is still there but the value
 * is now NULL it means we have already matched it with another peer.
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
static void find_one_peer(rpmfile_entry_t *file, rpmfile_t *after, struct file_data *after_table)
{
    struct file_data *entry = NULL;
    rpmfile_entry_t *after_file = NULL;
    const char *before_version = NULL;
    const char *after_version = NULL;
    bool has_version;
    const char *before_release = NULL;
    const char *after_release = NULL;
    char *before_tmp = NULL;
    char *after_tmp = NULL;
    char *search_path = NULL;
    const char *arch = NULL;
    const char *after_arch = NULL;

    assert(file != NULL);
    assert(after != NULL);
    assert(after_table != NULL);

    /* used in a number of matching checks below */
    after_file = TAILQ_FIRST(after);

    /* Start with the obvious case: the paths match */
    HASH_FIND_STR(after_table, file->localpath, entry);

    if (entry) {
        set_peer(file, entry);
        return;
    }

    /* Try substituting the version strings */
    before_version = headerGetString(file->rpm_header, RPMTAG_VERSION);
    after_version = headerGetString(after_file->rpm_header, RPMTAG_VERSION);

    /* If the path doesn't have a version in it we can skip these substitutions */
    has_version = (strstr(file->localpath, before_version) != NULL);

    if (has_version && (strcmp(before_version, after_version) != 0)) {
        search_path = strreplace(file->localpath, before_version, after_version);
        HASH_FIND_STR(after_table, search_path, entry);
        free(search_path);

        if (entry) {
            set_peer(file, entry);
            return;
        }
    }

    /* Try substituting version-release */
    if (has_version) {
        before_release = headerGetString(file->rpm_header, RPMTAG_RELEASE);
        after_release = headerGetString(after_file->rpm_header, RPMTAG_RELEASE);

        xasprintf(&before_tmp, "%s-%s", before_version, before_release);
        xasprintf(&after_tmp, "%s-%s", after_version, after_release);

        if (strcmp(before_tmp, after_tmp) != 0) {
            search_path = strreplace(file->localpath, before_tmp, after_tmp);

            free(before_tmp);
            free(after_tmp);

            HASH_FIND_STR(after_table, search_path, entry);
            free(search_path);

            if (entry) {
                set_peer(file, entry);
                return;
            }
        } else {
            free(before_tmp);
            free(after_tmp);
        }
    }

    /* See if this file peer moved */
    if (file->peer_file == NULL && S_ISREG(file->st.st_mode)) {
        /* .build-id files can be ignored, they always move */
        if (strstr(file->localpath, BUILD_ID_DIR)) {
            return;
        }

        /* we need the build architecture of the file */
        arch = get_rpm_header_arch(file->rpm_header);
        assert(arch != NULL);

        /* look for a possible match for files that move locations */
        TAILQ_FOREACH(after_file, after, items) {
            /* skip files with peers */
            if (after_file->peer_file) {
                continue;
            }

            /* if the build architectures differ, skip */
            after_arch = get_rpm_header_arch(after_file->rpm_header);
            assert(after_arch != NULL);

            if (strcmp(arch, after_arch)) {
                continue;
            }

            /* match files that move between subpackages */
            if (strsuffix(after_file->localpath, file->localpath) &&
                !strcmp(get_mime_type(file), get_mime_type(after_file)) &&
                strcmp(headerGetString(file->rpm_header, RPMTAG_NAME), headerGetString(after_file->rpm_header, RPMTAG_NAME))) {
                /*
                 * This is a best guess that checks the following:
                 * - localpath
                 * - MIME type
                 *
                 * This may need refinement down the road to check other things.
                 */
                DEBUG_PRINT("%s probably moved to %s\n", file->localpath, after_file->localpath);

                HASH_FIND_STR(after_table, after_file->localpath, entry);

                if (entry) {
                    set_peer(file, entry);
                    DEBUG_PRINT("moved subpackage\n");
                    file->moved_subpackage = true;
                    file->peer_file->moved_subpackage = true;
                    return;
                }
            } else if ((S_ISREG(file->st.st_mode) && S_ISREG(after_file->st.st_mode)) ||
                       (is_elf(file->fullpath) && is_elf(after_file->fullpath))) {
                /*
                 * Try to match libraries that have changed versions.
                 * The idea is to look for ELF files that carry a
                 * '.so.*' substring and then soft match.  Care has to
                 * be taken to ensure '.so.1' does not match up with
                 * '.so.2.0', so some things like counting periods
                 * will probably have to be done.
                 *
                 * Also try to match kernel modules between builds.
                 */
                if (!(strstr(file->localpath, ELF_LIB_EXTENSION) && strstr(after_file->localpath, ELF_LIB_EXTENSION)) &&
                    !(strstr(file->fullpath, KERNEL_MODULES_DIR) && strstr(after_file->fullpath, KERNEL_MODULES_DIR))) {
                    continue;
                }

                /* create generic version number paths */
                before_tmp = comparable_version_substrings(file->localpath, arch);
                after_tmp = comparable_version_substrings(after_file->localpath, arch);

                /* see if these generic paths match */
                if (!strcmp(before_tmp, after_tmp)) {
                    DEBUG_PRINT("%s probably replaced by %s\n", file->localpath, after_file->localpath);
                    HASH_FIND_STR(after_table, after_file->localpath, entry);

                    if (entry) {
                        set_peer(file, entry);
                    }
                }

                free(before_tmp);
                free(after_tmp);
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
    struct file_data *after_table = NULL;
    struct file_data *entry = NULL;
    struct file_data *tmp_entry = NULL;
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

    /* Clean up the hash table */
    HASH_ITER(hh, after_table, entry, tmp_entry) {
        HASH_DEL(after_table, entry);
        free(entry);
    }

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
        warn("open()");
        return NULL;
    }

    file->cap = cap_get_fd(fd);

    if (close(fd) == -1) {
        warn("close()");
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
