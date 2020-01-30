/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
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
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool dt_needed_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *bv = NULL;
    const char *av = NULL;
    const char *arch = NULL;
    int after_fd = -1;
    int before_fd = -1;
    Elf *after_elf = NULL;
    Elf *before_elf = NULL;
    Elf64_Half after_type;
    Elf64_Half before_type;
    size_t i;
    GElf_Dyn *dyn = NULL;
    size_t sz = 0;
    GElf_Shdr shdr;
    string_list_t *after_needed = NULL;
    string_list_t *before_needed = NULL;
    string_list_t *removed = NULL;
    string_list_t *added = NULL;
    string_entry_t *entry = NULL;
    char *msg = NULL;
    char *tmp = NULL;
    char *dump = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Skip files without a peer, other inspections handle new/missing files */
    if (!file->peer_file) {
        return true;
    }

    /* Only perform checks on regular files */
    if (!S_ISREG(file->st.st_mode)) {
        return true;
    }

    /* Skip files in the debug path and debug source path */
    if (strprefix(file->localpath, DEBUG_PATH) ||
        strprefix(file->localpath, DEBUG_SRC_PATH)) {
        return true;
    }

    /* Only run this inspection for builds that do not change versions */
    bv = headerGetString(file->peer_file->rpm_header, RPMTAG_VERSION);
    av = headerGetString(file->rpm_header, RPMTAG_VERSION);

    if (strcmp(bv, av)) {
      return true;
    }

    /* The architecture is used in reporting messages */
    arch = get_rpm_header_arch(file->rpm_header);
    assert(arch != NULL);

    /* If we lack dynamic or shared ELF files, we're done */
    if ((after_elf = get_elf(file->fullpath, &after_fd)) == NULL) {
        return true;
    }

    after_type = get_elf_type(after_elf);

    if (after_type != ET_DYN) {
        result = false;
        goto done;
    }

    if ((before_elf = get_elf(file->fullpath, &before_fd)) == NULL) {
        xasprintf(&msg, "%s was an ELF file and now is not on %s", file->localpath, arch);
        add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DT_NEEDED, msg, NULL, REMEDY_DT_NEEDED);
        free(msg);
        result = false;
        goto done;
    }

    before_type = get_elf_type(before_elf);

    if (before_type != ET_EXEC && before_type != ET_DYN) {
        xasprintf(&msg, "%s was a dynamic ELF file and now is not on %s", file->localpath, arch);
        add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DT_NEEDED, msg, NULL, REMEDY_DT_NEEDED);
        free(msg);
        result = false;
        goto done;
    }

    /* Gather the DT_NEEDED entries */
    if (get_dynamic_tags(after_elf, DT_NEEDED, &dyn, &sz, &shdr)) {
        after_needed = calloc(1, sizeof(*after_needed));
        assert(after_needed != NULL);
        TAILQ_INIT(after_needed);

        for (i = 0; i < sz; i++) {
            entry = calloc(1, sizeof(*entry));
            entry->data = strdup(elf_strptr(after_elf, shdr.sh_link, (size_t) (dyn[i].d_un.d_ptr)));
            TAILQ_INSERT_TAIL(after_needed, entry, items);
        }

        free(dyn);
    }

    if (get_dynamic_tags(before_elf, DT_NEEDED, &dyn, &sz, &shdr)) {
        before_needed = calloc(1, sizeof(*before_needed));
        assert(before_needed != NULL);
        TAILQ_INIT(before_needed);

        for (i = 0; i < sz; i++) {
            entry = calloc(1, sizeof(*entry));
            entry->data = strdup(elf_strptr(before_elf, shdr.sh_link, (size_t) (dyn[i].d_un.d_ptr)));
            TAILQ_INSERT_TAIL(before_needed, entry, items);
        }

        free(dyn);
    }

    /* Figure out what symbol changes happened*/
    removed = list_difference(before_needed, after_needed);
    added = list_difference(after_needed, before_needed);

    /* Report out any findings */
    if (removed != NULL && !TAILQ_EMPTY(removed)) {
        xasprintf(&msg, "DT_NEEDED symbol(s) removed from %s on %s", file->localpath, arch);

        TAILQ_FOREACH(entry, removed, items) {
            xasprintf(&tmp, "%s\n", entry->data);
            free(dump);
            dump = tmp;
        }

        add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DT_NEEDED, msg, dump, REMEDY_DT_NEEDED);
        free(msg);
        free(dump);
        result = false;
    }

    if (added != NULL && !TAILQ_EMPTY(added)) {
        xasprintf(&msg, "DT_NEEDED symbol(s) added to %s on %s", file->localpath, arch);

        TAILQ_FOREACH(entry, added, items) {
            xasprintf(&tmp, "%s\n", entry->data);
            free(dump);
            dump = tmp;
        }

        add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_DT_NEEDED, msg, dump, REMEDY_DT_NEEDED);
        free(msg);
        free(dump);
        result = false;
    }

done:
    if (after_elf && after_fd != -1) {
        elf_end(after_elf);
        close(after_fd);
    }

    if (before_elf && before_fd != -1) {
        elf_end(before_elf);
        close(before_fd);
    }

    free(removed);
    free(added);
    list_free(before_needed, free);
    list_free(after_needed, free);

    return result;
}

/*
 * Main driver for the 'DT_NEEDED' inspection.
 */
bool inspect_dt_needed(struct rpminspect *ri) {
    bool result;

    assert(ri != NULL);

    /* run the DT_NEEDED test across all ELF files */
    result = foreach_peer_file(ri, dt_needed_driver);

    /* if everything was fine, just say so */
    if (result) {
        add_result(ri, RESULT_OK, NOT_WAIVABLE, HEADER_DT_NEEDED, NULL, NULL, NULL);
    }

    return result;
}
