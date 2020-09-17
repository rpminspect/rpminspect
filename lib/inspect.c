/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include "queue.h"
#include "rpminspect.h"
#include "inspect.h"

/*
 * Debugging mode toggle, set at runtime.
 */
bool debug_mode = false;

/*
 * Ensure the array of inspections is only defined once.
 */

struct inspect inspections[] = {
    /*
     * { INSPECT_TYPE (add to inspect.h),
     *   "short name",
     *   bool--true if for single build, false if before&after required,
     *   &function_pointer },
     *
     * NOTE: long descriptions are inspect.h and returned by inspection_desc()
     */
    { INSPECT_LICENSE,       "license",       true,  &inspect_license },
    { INSPECT_EMPTYRPM,      "emptyrpm",      true,  &inspect_emptyrpm },
    { INSPECT_LOSTPAYLOAD,   "lostpayload",   false, &inspect_lostpayload },
    { INSPECT_METADATA,      "metadata",      true,  &inspect_metadata },
    { INSPECT_MANPAGE,       "manpage",       true,  &inspect_manpage },
    { INSPECT_XML,           "xml",           true,  &inspect_xml },
    { INSPECT_ELF,           "elf",           true,  &inspect_elf },
    { INSPECT_DESKTOP,       "desktop",       true,  &inspect_desktop },
    { INSPECT_DISTTAG,       "disttag",       true,  &inspect_disttag },
    { INSPECT_SPECNAME,      "specname",      true,  &inspect_specname },
    { INSPECT_MODULARITY,    "modularity",    true,  &inspect_modularity },
    { INSPECT_JAVABYTECODE,  "javabytecode",  true,  &inspect_javabytecode },
    { INSPECT_CHANGEDFILES,  "changedfiles",  false, &inspect_changedfiles },
    { INSPECT_MOVEDFILES,    "movedfiles",    false, &inspect_movedfiles },
    { INSPECT_REMOVEDFILES,  "removedfiles",  false, &inspect_removedfiles },
    { INSPECT_ADDEDFILES,    "addedfiles",    false, &inspect_addedfiles },
    { INSPECT_UPSTREAM,      "upstream",      false, &inspect_upstream },
    { INSPECT_OWNERSHIP,     "ownership",     true,  &inspect_ownership },
    { INSPECT_SHELLSYNTAX,   "shellsyntax",   true,  &inspect_shellsyntax },
    { INSPECT_ANNOCHECK,     "annocheck",     true,  &inspect_annocheck },
    { INSPECT_DT_NEEDED,     "DT_NEEDED",     false, &inspect_dt_needed },
    { INSPECT_FILESIZE,      "filesize",      false, &inspect_filesize },
    { INSPECT_PERMISSIONS,   "permissions",   true,  &inspect_permissions },
    { INSPECT_CAPABILITIES,  "capabilities",  true,  &inspect_capabilities },
    { INSPECT_KMOD,          "kmod",          false, &inspect_kmod },
    { INSPECT_ARCH,          "arch",          false, &inspect_arch },
    { INSPECT_SUBPACKAGES,   "subpackages",   false, &inspect_subpackages },
    { INSPECT_CHANGELOG,     "changelog",     false, &inspect_changelog },
    { INSPECT_PATHMIGRATION, "pathmigration", true,  &inspect_pathmigration },
    { INSPECT_LTO,           "LTO",           true,  &inspect_lto },
    { INSPECT_SYMLINKS,      "symlinks",      true,  &inspect_symlinks },
    { INSPECT_FILES,         "files",         true,  &inspect_files },
    { INSPECT_TYPES,         "types",         false, &inspect_types },
    { INSPECT_ABIDIFF,       "abidiff",       false, &inspect_abidiff },
    { INSPECT_KMIDIFF,       "kmidiff",       false, &inspect_kmidiff },
    { INSPECT_CONFIG,        "config",        false, &inspect_config },
    { INSPECT_DOC,           "doc",           false, &inspect_doc },
    { 0, NULL, false, NULL }
};

/**
 * @brief Iterate over each file in each package in a build.
 *
 * Inspect each "after" file in each peer of an inspection.  If the
 * foreach_peer_file_func returns false for any file, the result will
 * be false.  foreach_peer_file_func is run on each file even if an
 * earlier file fails. This allows for multiple errors to be collected
 * for a single inspection.
 *
 * @param ri Pointer to the struct rpminspect used for the program.
 * @param callback Callback function to iterate over each file.
 * @param use_ignore True to skip files that match entries in the
 *        ignore section of the configuration file, false otherwise.
 * @return True if the check_fn passed for each file, false otherwise.
 */
bool foreach_peer_file(struct rpminspect *ri, foreach_peer_file_func check_fn, bool use_ignore)
{
    rpmpeer_entry_t *peer;
    rpmfile_entry_t *file;
    bool result = true;

    assert(ri != NULL);
    assert(check_fn != NULL);

    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are caught by INSPECT_EMPTYRPM */
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            /* Ignore files we should be ignoring */
            if (use_ignore && ignore_path(ri, file->localpath, peer->after_root)) {
                continue;
            }

            if (!check_fn(ri, file)) {
                result = false;
            }
        }
    }

    return result;
}

/**
 * @brief Return inspection description string given its ID.
 *
 * Return the long description for the specified inspection.
 *
 * @param id Inspection ID constant.
 */
const char *inspection_desc(const uint64_t inspection)
{
    switch (inspection) {
        case INSPECT_LICENSE:
            return DESC_LICENSE;
        case INSPECT_EMPTYRPM:
            return DESC_EMPTYRPM;
        case INSPECT_LOSTPAYLOAD:
            return DESC_LOSTPAYLOAD;
        case INSPECT_METADATA:
            return DESC_METADATA;
        case INSPECT_MANPAGE:
            return DESC_MANPAGE;
        case INSPECT_XML:
            return DESC_XML;
        case INSPECT_ELF:
            return DESC_ELF;
        case INSPECT_DESKTOP:
            return DESC_DESKTOP;
        case INSPECT_DISTTAG:
            return DESC_DISTTAG;
        case INSPECT_SPECNAME:
            return DESC_SPECNAME;
        case INSPECT_MODULARITY:
            return DESC_MODULARITY;
        case INSPECT_JAVABYTECODE:
            return DESC_JAVABYTECODE;
        case INSPECT_CHANGEDFILES:
            return DESC_CHANGEDFILES;
        case INSPECT_MOVEDFILES:
            return DESC_MOVEDFILES;
        case INSPECT_REMOVEDFILES:
            return DESC_REMOVEDFILES;
        case INSPECT_ADDEDFILES:
            return DESC_ADDEDFILES;
        case INSPECT_UPSTREAM:
            return DESC_UPSTREAM;
        case INSPECT_OWNERSHIP:
            return DESC_OWNERSHIP;
        case INSPECT_SHELLSYNTAX:
            return DESC_SHELLSYNTAX;
        case INSPECT_ANNOCHECK:
            return DESC_ANNOCHECK;
        case INSPECT_DT_NEEDED:
            return DESC_DT_NEEDED;
        case INSPECT_FILESIZE:
            return DESC_FILESIZE;
        case INSPECT_PERMISSIONS:
            return DESC_PERMISSIONS;
        case INSPECT_CAPABILITIES:
            return DESC_CAPABILITIES;
        case INSPECT_KMOD:
            return DESC_KMOD;
        case INSPECT_ARCH:
            return DESC_ARCH;
        case INSPECT_SUBPACKAGES:
            return DESC_SUBPACKAGES;
        case INSPECT_CHANGELOG:
            return DESC_CHANGELOG;
        case INSPECT_PATHMIGRATION:
            return DESC_PATHMIGRATION;
        case INSPECT_LTO:
            return DESC_LTO;
        case INSPECT_SYMLINKS:
            return DESC_SYMLINKS;
        case INSPECT_FILES:
            return DESC_FILES;
        case INSPECT_TYPES:
            return DESC_TYPES;
        case INSPECT_ABIDIFF:
            return DESC_ABIDIFF;
        case INSPECT_KMIDIFF:
            return DESC_KMIDIFF;
        case INSPECT_CONFIG:
            return DESC_CONFIG;
        case INSPECT_DOC:
            return DESC_DOC;
        default:
            return NULL;
    }
}
