/*
 * Copyright © 2020 Red Hat, Inc.
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>
#include <limits.h>
#include <libgen.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>
#include "queue.h"
#include "rpminspect.h"

/* Globals */
static char *cmdprefix = NULL;
static string_list_t *suppressions = NULL;
static string_list_map_t *debug_info_dir1_table;
static string_list_map_t *debug_info_dir2_table;
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
    char **argv = NULL;
    string_entry_t *entry = NULL;
    const char *arch = NULL;
    const char *name = NULL;
    string_list_map_t *hentry = NULL;
    char *kabi = NULL;
    int exitcode = 0;
    int i = 0;
    char *fname[] = KERNEL_FILENAMES;
    char *compare = NULL;
    char *cmd = NULL;
    char *output = NULL;
    struct result_params params;
    bool report = false;

    assert(ri != NULL);
    assert(file != NULL);
    assert(cmdprefix != NULL);

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
    cmd = strdup(cmdprefix);
    assert(cmd != NULL);

    if (kabi) {
        cmd = strappend(cmd, " ", KMIDIFF_KMI_WHITELIST, " ", kabi, NULL);
        free(kabi);
    }

    if (suppressions && !TAILQ_EMPTY(suppressions)) {
        TAILQ_FOREACH(entry, suppressions, items) {
            cmd = strappend(cmd, " ", entry->data, NULL);
        }
    }

    /* debug dir args */
    HASH_FIND_STR(debug_info_dir1_table, arch, hentry);

    if (hentry != NULL && hentry->value && !TAILQ_EMPTY(hentry->value)) {
        TAILQ_FOREACH(entry, hentry->value, items) {
            cmd = strappend(cmd, " ", entry->data, NULL);
        }
    }

    HASH_FIND_STR(debug_info_dir2_table, arch, hentry);

    if (hentry != NULL && hentry->value && !TAILQ_EMPTY(hentry->value)) {
        TAILQ_FOREACH(entry, hentry->value, items) {
            cmd = strappend(cmd, " ", entry->data, NULL);
        }
    }

    /* the before and after kernel images and root directories */
    cmd = strappend(cmd, " ", KMIDIFF_VMLINUX1, " ", file->peer_file->fullpath, " ", KMIDIFF_VMLINUX2, " ", file->fullpath, " ", before_root, " ", after_root, NULL);

    /* run kmidiff */
    argv = build_argv(cmd);
    output = run_cmd_vpe(&exitcode, ri->worksubdir, argv);
    free_argv(argv);

    /* determine if this is a rebase build */
    rebase = is_rebase(ri);

    /* report the results */
    init_result_params(&params);
    params.header = NAME_KMIDIFF;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.remedy = REMEDY_KMIDIFF;
    params.arch = arch;
    params.file = file->localpath;

    if ((exitcode & ABIDIFF_ERROR) || (exitcode & ABIDIFF_USAGE_ERROR)) {
        params.severity = RESULT_VERIFY;
        params.verb = VERB_FAILED;
        params.noun = _("kmidiff usage error");
        report = true;
    } else if (!rebase && (exitcode & ABIDIFF_ABI_CHANGE)) {
        params.severity = RESULT_VERIFY;
        params.verb = VERB_CHANGED;
        params.noun = _("KMI change in ${FILE} on ${ARCH}");
        report = true;
    } else if (!rebase && (exitcode & ABIDIFF_ABI_INCOMPATIBLE_CHANGE)) {
        params.severity = RESULT_BAD;
        params.verb = VERB_CHANGED;
        params.noun = _("KMI incompatible change in ${FILE} on ${ARCH}");
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

    if (report || (exitcode && output)) {
        if (exitcode && output) {
            params.msg = strdup(_("KMI comparison ended unexpectedly."));
            params.verb = VERB_FAILED;
            params.noun = _("kmidiff unexpected exit");
        }

        params.file = file->localpath;
        xasprintf(&params.details, _("Command: %s\n\n%s"), cmd, output);
        add_result(ri, &params);
        free(params.msg);
        free(params.details);
        result = false;
    }

    /* cleanup */
    free(cmd);
    free(output);

    return result;
}

/*
 * Main driver for the 'kmidiff' inspection.
 */
bool inspect_kmidiff(struct rpminspect *ri)
{
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
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
    if (ri->kmidiff_extra_args) {
        xasprintf(&cmdprefix, "%s %s", ri->commands.kmidiff, ri->kmidiff_extra_args);
    } else {
        cmdprefix = strdup(ri->commands.kmidiff);
    }

    /* run the main inspection */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are caught by INSPECT_EMPTYRPM */
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            /* Ignore files we should be ignoring */
            if (ignore_path(ri, NAME_KMIDIFF, file->localpath, peer->after_root)) {
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
    free(cmdprefix);
    list_free(suppressions, free);
    free(kabi_dir);
    free_argv_table(ri, debug_info_dir1_table);
    free_argv_table(ri, debug_info_dir2_table);

    /* report the inspection results */
    if (result) {
        init_result_params(&params);
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_KMIDIFF;
        params.severity = RESULT_OK;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
