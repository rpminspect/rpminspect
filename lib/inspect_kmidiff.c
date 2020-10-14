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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>
#include <limits.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>
#include "queue.h"
#include "rpminspect.h"

/* Globals */
static string_list_t *firstargs = NULL;
static string_list_t *suppressions = NULL;
static struct hsearch_data *debug_info_dir1_table;
static struct hsearch_data *debug_info_dir2_table;
static bool found_kernel_image = false;
static char *kabi_dir = NULL;

/* Pointers to root extracted package paths (do not free these) */
static const char *before_root = NULL;
static const char *after_root = NULL;

/**
 * Given a build, search for the dir path in all extracted packages.
 * If we don't find one, the path will remain NULL and no kabi will be
 * used during kmidiff runs.
 */
static void get_kabi_dir(struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    struct stat sbuf;

    assert(ri != NULL);

    /* nothing to do if kabi_path is not defined in the configuration */
    if (ri->kabi_dir == NULL) {
        return;
    }

    /* look for first matching path */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            /* use stat(2) here because we want to read what symlinks point to */
            if (stat(file->fullpath, &sbuf) != 0) {
                warn("stat()");
                continue;
            }

            if (!S_ISDIR(sbuf.st_mode)) {
                continue;
            }

            /* found the kabi directory in this package */
            if (!strcmp(file->localpath, ri->kabi_dir)) {
                kabi_dir = strdup(file->fullpath);
                break;
            }
        }

        if (kabi_dir) {
            break;
        }
    }

    return;
}

/*
 * Return a full path to the appropriate kabi file or NULL if it doesn't exist.
 * Caller must free the returned string.
 */
static char *get_kabi_file(const struct rpminspect *ri, const char *arch)
{
    char *template = NULL;
    char *kabi = NULL;

    assert(ri != NULL);

    if (kabi_dir == NULL || ri->kabi_filename == NULL) {
        return NULL;
    }

    /* build the template */
    assert(arch != NULL);
    xasprintf(&template, "%s/%s", kabi_dir, ri->kabi_filename);
    assert(template != NULL);

    /* replace variables */
    kabi = strreplace(template, "${ARCH}", arch);

    if (strstr(kabi, "$ARCH")) {
        free(kabi);
        kabi = strreplace(template, "$ARCH", arch);
    }

    /* let's see if this is actually a file */
    if (access(kabi, R_OK)) {
        warn("%s", kabi);
        free(kabi);
        kabi = NULL;
    }

    /* we're done */
    free(template);
    return kabi;
}

static bool kmidiff_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    bool rebase = false;
    string_list_t *argv = NULL;
    string_entry_t *entry = NULL;
    string_list_t *local_suppressions = NULL;
    string_list_t *local_d1 = NULL;
    string_list_t *local_d2 = NULL;
    string_list_t *local_h1 = NULL;
    string_list_t *local_h2 = NULL;
    string_list_t *arglist = NULL;
    const char *arch = NULL;
    const char *name = NULL;
    char *kabi = NULL;
    ENTRY e;
    ENTRY *eptr;
    int exitcode = 0;
    int status = 0;
    int i = 0;
    char *fname[] = KERNEL_FILENAMES;
    char *compare = NULL;
    char *tmp = NULL;
    char *cmd = NULL;
    char *output = NULL;
    char *details = NULL;
    struct result_params params;
    bool report = false;

    assert(ri != NULL);
    assert(file != NULL);
    assert(firstargs != NULL);

    /* skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* skip anything except debuginfo packages */
    name = headerGetString(file->rpm_header, RPMTAG_NAME);
    assert(name != NULL);

    if (!strsuffix(name, DEBUGINFO_SUFFIX)) {
        return true;
    }

    /* skip anything without a peer */
    if (file->peer_file == NULL) {
        return true;
    }

    /* skip anything that is not an ELF file */
    if (!S_ISREG(file->st.st_mode) || !is_elf(file->fullpath)) {
        return true;
    }

    /* skip anything that is not a kernel image */
    for (i = 0; fname[i] != NULL; i++) {
        xasprintf(&compare, "/%s", basename(fname[i]));
        assert(compare != NULL);

        if (strsuffix(file->localpath, compare)) {
            found_kernel_image = true;
        }

        free(compare);

        if (found_kernel_image) {
            break;
        }
    }

    if (!found_kernel_image) {
        return true;
    }

    /* get the package architecture */
    arch = get_rpm_header_arch(file->rpm_header);
    kabi = get_kabi_file(ri, arch);

    /* build the kmidiff command */
    argv = list_copy(firstargs);

    if (kabi) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        xasprintf(&entry->data, "%s %s", KMIDIFF_KMI_WHITELIST, kabi);
        assert(entry->data != NULL);
        TAILQ_INSERT_TAIL(argv, entry, items);
        free(kabi);
    }

    if (suppressions && !TAILQ_EMPTY(suppressions)) {
        local_suppressions = list_copy(suppressions);
        TAILQ_CONCAT(argv, local_suppressions, items);
    }

    /* debug dir args */
    e.key = (char *) arch;
    hsearch_r(e, FIND, &eptr, debug_info_dir1_table);

    if (eptr != NULL) {
        arglist = (string_list_t *) eptr->data;

        if (arglist && !TAILQ_EMPTY(arglist)) {
            local_d1 = list_copy(arglist);
            TAILQ_CONCAT(argv, local_d1, items);
        }
    }

    e.key = (char *) arch;
    hsearch_r(e, FIND, &eptr, debug_info_dir2_table);

    if (eptr != NULL) {
        arglist = (string_list_t *) eptr->data;

        if (arglist && !TAILQ_EMPTY(arglist)) {
            local_d2 = list_copy(arglist);
            TAILQ_CONCAT(argv, local_d2, items);
        }
    }

    /* the before kernel image */
    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "%s %s", KMIDIFF_VMLINUX1, file->peer_file->fullpath);
    assert(entry->data != NULL);
    TAILQ_INSERT_TAIL(argv, entry, items);

    /* the after kernel image */
    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "%s %s", KMIDIFF_VMLINUX2, file->fullpath);
    assert(entry->data != NULL);
    TAILQ_INSERT_TAIL(argv, entry, items);

    /* the before root */
    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    entry->data = strdup(before_root);
    assert(entry->data != NULL);
    TAILQ_INSERT_TAIL(argv, entry, items);

    /* the after root */
    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    entry->data = strdup(after_root);
    assert(entry->data != NULL);
    TAILQ_INSERT_TAIL(argv, entry, items);

    /* run kmidiff */
    output = sl_run_cmd(&exitcode, argv);

    /* generate a reporting string for the command run */
    cmd = list_to_string(argv, " ");
    tmp = strreplace(cmd, ri->worksubdir, NULL);
    assert(tmp != NULL);
    free(cmd);
    cmd = tmp;

    /* determine if this is a rebase build */
    rebase = is_rebase(ri);

    /* report the results */
    init_result_params(&params);
    params.header = HEADER_KMIDIFF;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.remedy = REMEDY_KMIDIFF;
    params.arch = arch;

    if (!WIFEXITED(exitcode)) {
        xasprintf(&details, _("Command: %s"), cmd);
        params.msg = _("KMI comparison ended unexpectedly.");
        params.verb = VERB_FAILED;
        params.noun = ri->commands.kmidiff;
        report = true;
    } else {
        status = WEXITSTATUS(exitcode);

        if ((status & ABIDIFF_ERROR) || (status & ABIDIFF_USAGE_ERROR)) {
            params.severity = RESULT_VERIFY;
            params.verb = VERB_FAILED;
            params.noun = ri->commands.kmidiff;
            report = true;
        }

        if (!rebase && (status & ABIDIFF_ABI_CHANGE)) {
            params.severity = RESULT_VERIFY;
            params.verb = VERB_CHANGED;
            params.noun = _("KMI");
            report = true;
        }

        if (!rebase && (status & ABIDIFF_ABI_INCOMPATIBLE_CHANGE)) {
            params.severity = RESULT_BAD;
            params.verb = VERB_CHANGED;
            params.noun = _("KMI");
            report = true;
        }

        /* check the ABI compat level list */
        name = headerGetString(file->rpm_header, RPMTAG_NAME);

        /* add additional details */
        if (report) {
            if (!strcmp(file->peer_file->localpath, file->localpath)) {
                xasprintf(&params.msg, _("Comparing old vs. new version of %s in package %s on %s revealed Kernel Module Interface (KMI) differences."), file->localpath, name, arch);
            } else {
                xasprintf(&params.msg, _("Comparing from %s to %s in package %s on %s revealed Kernel Module Interface (KMI) differences."), file->peer_file->localpath, file->localpath, name, arch);
            }
        }
    }

    if (report) {
        params.file = file->localpath;
        xasprintf(&params.details, _("Command: %s\n\n%s"), cmd, output);
        add_result(ri, &params);
        free(params.msg);
        result = false;
    }

    /* cleanup */
    list_free(argv, free);
    free(local_suppressions);
    free(local_d1);
    free(local_d2);
    free(local_h1);
    free(local_h2);
    free(cmd);
    free(details);
    free(output);

    return result;
}

/*
 * Main driver for the 'kmidiff' inspection.
 */
bool inspect_kmidiff(struct rpminspect *ri) {
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    string_entry_t *entry = NULL;
    size_t num_arches = 0;
    struct result_params params;

    assert(ri != NULL);

    /* get the kabi path if that exists in this build */
    get_kabi_dir(ri);

    /* if there's a .abignore file in the after SRPM, we need to use it */
    suppressions = get_abi_suppressions(ri, ri->kmidiff_suppression_file);

    /* get the debug info dirs (headers not used for kmidiff) */
    num_arches = list_len(ri->arches);
    debug_info_dir1_table = get_abi_dir_arg(ri, num_arches, DEBUGINFO_SUFFIX, ABI_DEBUG_INFO_DIR1, DEBUG_PATH, BEFORE_BUILD);
    debug_info_dir2_table = get_abi_dir_arg(ri, num_arches, DEBUGINFO_SUFFIX, ABI_DEBUG_INFO_DIR2, DEBUG_PATH, AFTER_BUILD);

    /* build the list of first command line arguments */
    firstargs = calloc(1, sizeof(*firstargs));
    assert(firstargs != NULL);
    TAILQ_INIT(firstargs);

    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    entry->data = strdup(ri->commands.kmidiff);
    TAILQ_INSERT_TAIL(firstargs, entry, items);

    if (ri->kmidiff_extra_args) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->data = strdup(ri->kmidiff_extra_args);
        TAILQ_INSERT_TAIL(firstargs, entry, items);
    }

    /* run the main inspection */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are caught by INSPECT_EMPTYRPM */
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            /* Ignore files we should be ignoring */
            if (ignore_path(ri, file->localpath, peer->after_root)) {
                continue;
            }

            before_root = peer->before_root;
            after_root = peer->after_root;

            if (!kmidiff_driver(ri, file)) {
                result = false;
            }

            if (found_kernel_image) {
                break;
            }
        }

        if (found_kernel_image) {
            break;
        }
    }

    /* clean up */
    list_free(firstargs, free);
    list_free(suppressions, free);
    free(kabi_dir);
    free_argv_table(ri, debug_info_dir1_table);
    free_argv_table(ri, debug_info_dir2_table);

    /* report the inspection results */
    if (result) {
        init_result_params(&params);
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_KMIDIFF;
        params.severity = RESULT_OK;
        add_result(ri, &params);
    }

    return result;
}
