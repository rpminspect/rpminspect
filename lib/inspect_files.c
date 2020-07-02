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

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/queue.h>
#include <rpm/header.h>
#include "rpminspect.h"

/* All of the macros that can appear in a %files section */
static const char *files_macros[] = { "%artifact",
                                      "%attr",
                                      "%config",
                                      "%defattr",
                                      "%defverify",
                                      "%dev",
                                      "%dir",
                                      "%doc",
                                      "%docdir",
                                      "%exclude",
                                      "%ghost",
                                      "%lang",
                                      "%license",
                                      "%missingok",
                                      "%pubkey",
                                      "%readme",
                                      "%verify",
                                      NULL };

static bool files_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    bool result = true;
    bool in_files = false;
    bool valid_macro = false;
    long line = 1;
    int i = 0;
    struct result_params params;
    string_list_t *spec = NULL;
    string_entry_t *specline = NULL;
    string_entry_t *path = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* if there are no forbidden paths defined, we pass */
    if (ri->forbidden_paths == NULL || TAILQ_EMPTY(ri->forbidden_paths)) {
        return true;
    }

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_VERIFY;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = HEADER_FILES;
    params.remedy = REMEDY_FILES;
    params.file = file->localpath;

    /* read in the spec file */
    spec = read_file(file->fullpath);
    assert(spec != NULL);

    /* scan all %files sections for any forbidden path references */
    TAILQ_FOREACH(specline, spec, items) {
        /* determine when we enter and leave %files sections */
        if (*specline->data == '%') {
            if (strprefix(specline->data, SPEC_SECTION_FILES)) {
                in_files = true;
            } else if (in_files) {
                /* look for valid macros in the %files section */
                for (i = 0; files_macros[i] != NULL; i++) {
                    if (strprefix(specline->data, files_macros[i])) {
                        valid_macro = true;
                        break;
                    }
                }

                if (!valid_macro && !strprefix(specline->data, "%{")) {
                    in_files = false;
                }
            }
        }

        /* check for forbidden references */
        if (in_files && *specline->data != '#') {
            TAILQ_FOREACH(path, ri->forbidden_paths, items) {
                if (strstr(specline->data, path->data)) {
                    xasprintf(&params.msg, _("Forbidden path reference (%s) on line %ld of %s"), path->data, line, file->localpath);
                    params.details = specline->data;
                    add_result(ri, &params);
                    free(params.msg);
                    result = false;
                }
            }
        }

        /* increment spec file line counter */
        line++;

        /* reset for next loop iteration--only valid for current line */
        valid_macro = false;
    }

    list_free(spec, free);
    return result;
}

/*
 * Main driver for the 'files' inspection.
 */
bool inspect_files(struct rpminspect *ri) {
    bool result = false;
    bool src = false;
    struct result_params params;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;

    assert(ri != NULL);

    /* find the after build SRPM */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (headerIsSource(peer->after_hdr)) {
            /* find the after build spec file */
            TAILQ_FOREACH(file, peer->after_files, items) {
                if (strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
                    src = true;
                    result = files_driver(ri, file);
                    break;
                }
            }
        }
    }

    init_result_params(&params);
    params.waiverauth = NOT_WAIVABLE;
    params.header = HEADER_FILES;

    if (result && src) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    } else if (!src) {
        params.severity = RESULT_INFO;
        xasprintf(&params.msg, _("The %%files inspection is only for source packages, skipping."));
        add_result(ri, &params);
        free(params.msg);
    }

    return result;
}
