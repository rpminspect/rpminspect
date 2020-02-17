/*
 * Copyright (C) 2020  Red Hat, Inc.
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

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool kmod_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    int err = -1;
    bool result_parm = true;
    bool result_deps = true;
    struct kmod_ctx *kctx = NULL;
    struct kmod_module *beforekmod = NULL;
    struct kmod_module *afterkmod = NULL;
    struct kmod_list *beforeinfo = NULL;
    struct kmod_list *afterinfo = NULL;
    string_list_t *lost = NULL;
    string_list_t *gain = NULL;
    string_entry_t *entry = NULL;
    const char *beforename = NULL;
    const char *aftername = NULL;
    const char *beforever = NULL;
    const char *afterver = NULL;
    severity_t sev = RESULT_INFO;
    waiverauth_t waiver = NOT_WAIVABLE;
    char *msg = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Only perform this inspection on regular files */
    if (!S_ISREG(file->st.st_mode)) {
        return true;
    }

    /* Skip files in the debug source path */
    if (strprefix(file->localpath, DEBUG_PATH)) {
        return true;
    }

    /* Restrict our view to files that match kernel module paths */
    if (!strstr(file->localpath, KERNEL_MODULES_DIR) || !strstr(file->localpath, KERNEL_MODULE_FILENAME_EXTENSION)) {
        /*
         * File is not in the known kernel module path or lacks the
         * expected extension (could be compressed too)
         */
        return true;
    }

    /* Set reporting conditions based on the package name and version */
    beforename = headerGetString(file->peer_file->rpm_header, RPMTAG_NAME);
    aftername = headerGetString(file->rpm_header, RPMTAG_NAME);
    beforever = headerGetString(file->peer_file->rpm_header, RPMTAG_VERSION);
    afterver = headerGetString(file->rpm_header, RPMTAG_VERSION);
    assert(beforename != NULL);
    assert(aftername != NULL);
    assert(beforever != NULL);
    assert(afterver != NULL);

    if (!strcmp(beforename, aftername) && !strcmp(afterver, beforever)) {
        /* Package name and version are the same, kmod params lost are bad */
        sev = RESULT_VERIFY;
        waiver = WAIVABLE_BY_ANYONE;
    }

    /* Read in the kernel modules */
    kctx = kmod_new(NULL, NULL);

    if (kctx == NULL) {
        fprintf(stderr, "*** kmod_new() failure\n");
        return false;
    }

    err = kmod_module_new_from_path(kctx, file->peer_file->fullpath, &beforekmod);
    if (err < 0) {
        /* not a kernel module */
        kmod_unref(kctx);
        return true;
    }

    kmod_unref(kctx);
    kctx = kmod_new(NULL, NULL);

    if (kctx == NULL) {
        fprintf(stderr, "*** kmod_new() failure\n");
        return false;
    }

    err = kmod_module_new_from_path(kctx, file->fullpath, &afterkmod);
    if (err < 0) {
        /* not a kernel module */
        kmod_module_unref(beforekmod);
        kmod_unref(kctx);
        return true;
    }

    /* Gather module parameters */
    err = kmod_module_get_info(beforekmod, &beforeinfo);
    if (err < 0) {
        fprintf(stderr, "*** error reading before kernel module %s\n", file->peer_file->fullpath);
        kmod_module_unref(beforekmod);
        kmod_module_unref(afterkmod);
        kmod_unref(kctx);
        return false;
    }

    err = kmod_module_get_info(afterkmod, &afterinfo);
    if (err < 0) {
        fprintf(stderr, "*** error reading after kernel module %s\n", file->peer_file->fullpath);
        kmod_module_info_free_list(beforeinfo);
        kmod_module_unref(beforekmod);
        kmod_module_unref(afterkmod);
        kmod_unref(kctx);
        return false;
    }

    /* Compute lost and gained module parameters */
    result_parm = compare_module_parameters(beforeinfo, afterinfo, &lost, &gain);

    /* Report parameters */
    if (lost != NULL && !TAILQ_EMPTY(lost)) {
        TAILQ_FOREACH(entry, lost, items) {
            xasprintf(&msg, "Kernel module %s removes parameter '%s'", file->localpath, entry->data);
            add_result(ri, sev, waiver, HEADER_KMOD, msg, NULL, REMEDY_KMOD_PARM);
            free(msg);
        }

        list_free(lost, free);
        lost = NULL;
    }

    if (gain != NULL && !TAILQ_EMPTY(gain)) {
        TAILQ_FOREACH(entry, gain, items) {
            xasprintf(&msg, "Kernel module %s adds parameter '%s'", file->localpath, entry->data);
            add_result(ri, RESULT_INFO, NOT_WAIVABLE, HEADER_KMOD, msg, NULL, NULL);
            free(msg);
        }

        list_free(gain, free);
        gain = NULL;
    }

    /* Compute lost and gained module dependencies */
    result_deps = compare_module_dependencies(beforeinfo, afterinfo, &lost, &gain);

    /* Report dependencies */
    if (lost != NULL && !TAILQ_EMPTY(lost)) {
        TAILQ_FOREACH(entry, lost, items) {
            xasprintf(&msg, "Kernel module %s removes dependency '%s'", file->localpath, entry->data);
            add_result(ri, sev, waiver, HEADER_KMOD, msg, NULL, REMEDY_KMOD_DEPS);
            free(msg);
        }

        list_free(lost, free);
        lost = NULL;
    }

    if (gain != NULL && !TAILQ_EMPTY(gain)) {
        TAILQ_FOREACH(entry, gain, items) {
            xasprintf(&msg, "Kernel module %s adds dependency '%s'", file->localpath, entry->data);
            add_result(ri, sev, waiver, HEADER_KMOD, msg, NULL, REMEDY_KMOD_DEPS);
            free(msg);
        }

        list_free(gain, free);
        gain = NULL;
    }

    /* Clean up libkmod usage */
    kmod_module_info_free_list(beforeinfo);
    kmod_module_info_free_list(afterinfo);
    kmod_module_unref(beforekmod);
    kmod_module_unref(afterkmod);
    kmod_unref(kctx);

    return result_parm && result_deps;
}

/*
 * Main driver for the 'kmod' inspection.
 */
bool inspect_kmod(struct rpminspect *ri) {
    bool result;

    assert(ri != NULL);

    /* run the kmod inspection across all RPM files */
    result = foreach_peer_file(ri, kmod_driver);

    /* if everything was fine, just say so */
    if (result) {
        add_result(ri, RESULT_OK, NOT_WAIVABLE, HEADER_KMOD, NULL, NULL, NULL);
    }

    return result;
}
