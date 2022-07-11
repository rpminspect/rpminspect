/*
 * Copyright 2020 Red Hat, Inc.
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
#include <errno.h>
#include <err.h>
#include <limits.h>
#include <ctype.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>
#include <archive.h>
#include "queue.h"
#include "rpminspect.h"

/* Global variables */
static patches_t *patches = NULL;
static applied_patches_t *applied = NULL;
static bool reported = false;
static bool comparison = false;
static bool automacro = false;

enum { DIFF_NULL, DIFF_CONTEXT, DIFF_UNIFIED };

/*
 * Free the patches hash table
 */
static void free_patches(patches_t *table)
{
    patches_t *entry = NULL;
    patches_t *tmp_entry = NULL;

    if (table == NULL) {
        return;
    }

    HASH_ITER(hh, table, entry, tmp_entry) {
        HASH_DEL(table, entry);
        free(entry->patch);
        free(entry);
    }

    return;
}

/*
 * Free the applied patches hash table
 */
static void free_applied_patches(applied_patches_t *table)
{
    applied_patches_t *entry = NULL;
    applied_patches_t *tmp_entry = NULL;

    if (table == NULL) {
        return;
    }

    HASH_ITER(hh, table, entry, tmp_entry) {
        HASH_DEL(table, entry);
        free(entry->opts);
        free(entry);
    }

    return;
}

/*
 * Returns true if the %autopatch or %autosetup macros are in use in
 * the spec file.
 */
static bool have_automacro(const rpmfile_entry_t *specfile)
{
    bool r = false;
    bool in_valid_section = false;
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    char *buf = NULL;

    /* No spec file, we know nothing. */
    if (specfile == NULL) {
        return false;
    }

    /* Read in the spec file */
    contents = read_file(specfile->fullpath);

    if (contents == NULL) {
        return false;
    }

    /* Look for %autopatch or %autosetup in valid sections */
    TAILQ_FOREACH(entry, contents, items) {
        if (entry->data == NULL) {
            continue;
        }

        buf = entry->data;

        /* trim line endings */
        buf[strcspn(buf, "\r\n")] = 0;

        /* we made it to the changelog, nothing left of value */
        if (strprefix(buf, SPEC_SECTION_CHANGELOG)) {
            break;
        }

        /* section marker check */
        /*
         * not sure if leading whitespace is allowed by RPM, but I
         * have seen stranger things
         */
        buf = strtrim(buf);

        if (buf && *buf == '%') {
            if (strprefix(buf, SPEC_SECTION_PREP)
                || strprefix(buf, SPEC_SECTION_BUILD)
                || strprefix(buf, SPEC_SECTION_INSTALL)
                || strprefix(buf, SPEC_SECTION_CHECK)) {
                in_valid_section = true;
            } else if (!strcmp(buf, SPEC_SECTION_CHANGELOG)) {
                in_valid_section = false;
                break;
            }
        }

        /* look for the auto macros */
        /*
         * this matches lines that are either the macro itself, or the
         * macro followed by one or more options
         */
        if (in_valid_section && buf && (!strcmp(buf, SPEC_MACRO_AUTOPATCH) || strprefix(buf, SPEC_MACRO_AUTOPATCH" ")
                                        || !strcmp(buf, SPEC_MACRO_AUTOSETUP) || strprefix(buf, SPEC_MACRO_AUTOSETUP" "))) {
            r = true;
            break;
        }
    }

    list_free(contents, free);
    return r;
}

/* Returns true if this file is a Patch file */
static bool is_patch(const rpmfile_entry_t *file)
{
    char *shortname = NULL;
    patches_t *hentry = NULL;

    assert(file != NULL);

    if (patches == NULL) {
        return false;
    }

    /* The RPM header stores basenames */
    shortname = rindex(file->fullpath, '/') + 1;

    /* See if this file is a Patch file */
    HASH_FIND_STR(patches, shortname, hentry);

    if (hentry) {
        return true;
    } else {
        return false;
    }
}

/*
 * Compute number of files and lines changed in a patch.
 */
static patchstat_t get_patch_stats(const char *patch)
{
    patchstat_t r;
    string_list_t *lines = NULL;
    string_entry_t *line = NULL;
    int difftype = DIFF_NULL;
    bool maybe_context = false;
    bool maybe_unified = false;
    int header_count = -1;

    assert(patch != NULL);

    /* initialize our counts */
    r.files = 0;
    r.lines = 0;

    /* read in the patch file */
    lines = read_file(patch);

    if (lines == NULL || TAILQ_EMPTY(lines)) {
        return r;
    }

    /* iterate over each line counting files and line modifications */
    TAILQ_FOREACH(line, lines, items) {
        if (difftype == DIFF_NULL) {
            if (!maybe_context && !maybe_unified) {
                if (strprefix(line->data, "*** ")) {
                    header_count++;
                    maybe_context = true;
                } else if (strprefix(line->data, "--- ")) {
                    header_count++;
                    maybe_unified = true;
                }
            } else if (maybe_context && !maybe_unified) {
                if (strprefix(line->data, "--- ")) {
                    header_count++;
                } else if (header_count == 1 && strprefix(line->data, "**********")) {
                    r.files++;
                    difftype = DIFF_CONTEXT;
                    header_count = -1;
                    maybe_context = false;
                }
            } else if (!maybe_context && maybe_unified) {
                if (strprefix(line->data, "+++ ")) {
                    header_count++;
                } else if (header_count == 1 && strprefix(line->data, "@@ ")) {
                    r.files++;
                    difftype = DIFF_UNIFIED;
                    header_count = -1;
                    maybe_unified = false;
                }
            }
        } else if (difftype == DIFF_CONTEXT) {
            if (strprefix(line->data, "+ ") || strprefix(line->data, "- ")) {
                r.lines++;
            } else if (strprefix(line->data, "*** ")) {
                difftype = DIFF_NULL;
                header_count++;
                maybe_context = true;
            }
        } else if (difftype == DIFF_UNIFIED) {
            if ((strprefix(line->data, "+") || strprefix(line->data, "-")) && !strprefix(line->data, "--- ")) {
                r.lines++;
            } else if (strprefix(line->data, "--- ")) {
                difftype = DIFF_NULL;
                header_count++;
                maybe_unified = true;
            }
        }
    }

    /* clean up */
    list_free(lines, free);

    return r;
}

/* Main driver for the 'patches' inspection. */
static bool patches_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    patches_t *pentry = NULL;
    applied_patches_t *aentry = NULL;
    char *buf = NULL;
    char *before_patch = NULL;
    char *after_patch = NULL;
    char *details = NULL;
    patchstat_t ps;
    struct stat sb;
    size_t apsz = 0;
    size_t bpsz = 0;
    long unsigned int oldsize = 0;
    long unsigned int newsize = 0;
    struct result_params params;

    /* If we are not looking at a Patch file, bail. */
    if (!is_patch(file)) {
        return true;
    }

    /* If this patch is on the ignore list, skip */
    if (list_contains(ri->patch_ignore_list, file->localpath)) {
        DEBUG_PRINT("Per the configuration file, ignoring %s\n", file->localpath);
        return true;
    }

    init_result_params(&params);
    params.header = NAME_PATCHES;

    /* make sure defined patches are all applied */
    if (!automacro) {
        /* patches are defined without leading directories */
        buf = file->localpath;

        while (*buf == '/' && *buf != '\0') {
            buf++;
        }

        /* first look to see if the patch is in the header */
        HASH_FIND_STR(patches, buf, pentry);

        /* if it is defined, now try to find its apply macro */
        if (pentry != NULL) {
            HASH_FIND_INT(applied, &(pentry->num), aentry);

            /* a defined patch without an apply macro is a problem */
            if (aentry == NULL) {
                xasprintf(&params.msg, _("Patch number %ld (%s) is missing a corresponding %%patch%ld macro, usually in %%prep."), pentry->num, pentry->patch, pentry->num);
                params.remedy = REMEDY_PATCHES_MISSING_MACRO;
                params.noun = _("missing %%patch macro for ${FILE}");
            } else if (pentry->num != aentry->num) {
                xasprintf(&params.msg, _("Patch number %ld (%s) is mismatched with %%patch%ld macro."), pentry->num, pentry->patch, pentry->num);
                params.remedy = REMEDY_PATCHES_MISMATCHED_MACRO;
                params.noun = _("mismatched %%patch macro for ${FILE}");
            }

            if (params.msg) {
                params.severity = RESULT_VERIFY;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.details = NULL;
                params.verb = VERB_FAILED;
                params.file = file->localpath;
                add_result(ri, &params);
                free(params.msg);
                params.msg = NULL;
                reported = true;
            }
        } else {
            /* we have no patch defined for this patch file -- likely unreachable */
            params.severity = RESULT_BAD;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.details = NULL;
            params.verb = VERB_FAILED;
            params.file = file->localpath;
            params.noun = _("undefined Patch ${FILE}");
            xasprintf(&params.msg, _("Undefined Patch file %s."), file->localpath);
            add_result(ri, &params);
            free(params.msg);
            params.msg = NULL;
            reported = true;
        }
    }

    /* patches may be compressed, so uncompress them here for diff(1) */
    if (file->peer_file) {
        before_patch = uncompress_file(ri, file->peer_file->fullpath, NAME_PATCHES);

        if (before_patch == NULL) {
            warnx(_("unable to uncompress patch: %s"), file->peer_file->localpath);
            return false;
        }
    }

    after_patch = uncompress_file(ri, file->fullpath, NAME_PATCHES);

    if (after_patch == NULL) {
        warnx(_("unable to uncompress patch: %s"), file->localpath);
        return false;
    }

    /*
     * Ensure that all patches are at least 4 bytes in size, trapping
     * "empty patch" mistakes that have occurred when people are
     * generating multiple patches against multiple branches.
     */
    if (stat(after_patch, &sb) != 0) {
        warn("stat");
        return false;
    }

    apsz = sb.st_size;

    if (file->peer_file && before_patch && stat(before_patch, &sb) != 0) {
        warn("stat");
        return false;
    }

    bpsz = sb.st_size;

    if ((apsz < 4) || (bpsz < 4)) {
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.details = NULL;
        params.remedy = REMEDY_PATCHES_CORRUPT;
        params.verb = VERB_FAILED;
        params.noun = _("corrupt patch ${FILE}");

        if (apsz < 4) {
            params.file = file->localpath;
            xasprintf(&params.msg, _("Patch %s is under 4 bytes in size - is it corrupt?"), file->localpath);
            add_result(ri, &params);
            free(params.msg);
        }

        if (file->peer_file && before_patch && bpsz < 4) {
            params.file = file->peer_file->localpath;
            xasprintf(&params.msg, _("Patch %s is under 4 bytes in size - is it corrupt?"), file->peer_file->localpath);
            add_result(ri, &params);
            free(params.msg);
        }

        reported = true;
        free(after_patch);
        free(before_patch);
        return false;
    }

    /*
     * compare the patches if we have two builds
     * This just reports patches that change content.  It uses the INFO reporting level.
     */
    if (comparison && file->peer_file) {
        params.details = get_file_delta(before_patch, after_patch);

        if (params.details) {
            /* more than whitespace changed */
            oldsize = file->peer_file->st.st_size;
            newsize = file->st.st_size;
            xasprintf(&params.msg, _("%s changed (%ld bytes -> %ld bytes)"), file->localpath, oldsize, newsize);
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
            params.verb = VERB_CHANGED;
            params.noun = _("patch file ${FILE}");
            params.file = file->localpath;

            /* use friendly names for the files in the diff(1) details */
            details = strreplace(params.details, before_patch, file->peer_file->localpath);
            assert(details != NULL);
            free(params.details);
            params.details = strreplace(details, after_patch, file->localpath);
            free(details);
            assert(params.details != NULL);

            /* report the findings */
            add_result(ri, &params);
            free(params.details);
            free(params.msg);
            params.details = NULL;
            params.msg = NULL;

            reported = true;
        }
    } else if (comparison && file->peer_file == NULL) {
        xasprintf(&params.msg, _("New patch file `%s` appeared"), params.file);
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.verb = VERB_ADDED;
        params.noun = _("patch file ${FILE}");
        params.file = file->localpath;
        add_result(ri, &params);
        free(params.msg);
        params.msg = NULL;

        reported = true;
    }

    /*
     * Collect patch stats and report based on thresholds.
     */
    ps = get_patch_stats(after_patch);
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.verb = VERB_CHANGED;
    params.noun = _("patch changes ${FILE}");
    params.file = file->localpath;

    if (ps.files == 0 && ps.lines > 0) {
        xasprintf(&params.msg, _("%s touches as many as %ld line%s"), file->localpath, ps.lines, (ps.lines > 1) ? "s" : "");
    } else if (ps.files > 0 && ps.lines == 0) {
        xasprintf(&params.msg, _("%s touches %ld file%s"), file->localpath, ps.files, (ps.files > 1) ? "s" : "");
    } else {
        xasprintf(&params.msg, _("%s touches %ld file%s and %s%ld line%s"), file->localpath, ps.files, (ps.files > 1) ? "s" : "", (ps.lines > 1) ? _("as many as ") : "", ps.lines, (ps.lines > 1) ? "s" : "");
    }

    add_result(ri, &params);
    free(params.msg);
    params.msg = NULL;

    reported = true;

    /* clean up */
    free(before_patch);
    free(after_patch);

    return true;
}

/*
 * Main driver for the 'patches' inspection.
 */
bool inspect_patches(struct rpminspect *ri)
{
    bool result = true;
    bool have_source = false;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    rpmfile_entry_t *specfile = NULL;
    string_list_t *patchfiles = NULL;
    string_entry_t *patch = NULL;
    string_list_t *speclines = NULL;
    string_entry_t *specentry = NULL;
    string_list_t *fields = NULL;
    string_entry_t *entry = NULL;
    patches_t *hentry = NULL;
    applied_patches_t *aentry = NULL;
    char *buf = NULL;
    size_t len = 0;
    struct result_params params;

    assert(ri != NULL);

    init_result_params(&params);
    params.header = NAME_PATCHES;

    /* Check for source package */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (headerIsSource(peer->after_hdr)) {
            have_source = true;

            if (peer->before_hdr && headerIsSource(peer->before_hdr)) {
                comparison = true;
            }

            break;
        }
    }

    /* If no source found, we are not looking at source packages */
    if (!have_source) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        xasprintf(&params.msg, _("No source packages available, skipping inspection."));
        add_result(ri, &params);
        free(params.msg);
        reported = true;
        return result;
    }

    /* Set default parameters */
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.remedy = NULL;

    /* Run the main inspection */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Only look at the files in SRPMs */
        if (!headerIsSource(peer->after_hdr)) {
            continue;
        }

        /* On the off chance the SRPM is empty, just ignore */
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        /* Get the spec file */
        TAILQ_FOREACH(file, peer->after_files, items) {
            if (strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
                specfile = file;
                break;
            }
        }

        /* Determine if %autopatch or %autosetup is used */
        automacro = have_automacro(specfile);

        /* Initialize the patches hash table */
        patchfiles = get_rpm_header_string_array(specfile->rpm_header, RPMTAG_PATCH);

        if (patchfiles != NULL && !TAILQ_EMPTY(patchfiles)) {
            /* get patch numbers unless automacro is in use */
            if (automacro) {
                TAILQ_FOREACH(patch, patchfiles, items) {
                    hentry = calloc(1, sizeof(*hentry));
                    assert(hentry != NULL);
                    hentry->patch = strdup(patch->data);
                    hentry->num = -1;                       /* automacro == true */
                    HASH_ADD_KEYPTR(hh, patches, hentry->patch, strlen(hentry->patch), hentry);
                }
            } else {
                /* read in the spec file */
                speclines = read_file(file->fullpath);

                if (speclines == NULL) {
                    err(RI_PROGRAM_ERROR, "read_contents");
                }

                TAILQ_FOREACH(specentry, speclines, items) {
                    fields = NULL;

                    /* no more patch files to check */
                    if (patchfiles == NULL || TAILQ_EMPTY(patchfiles)) {
                        break;
                    }

                    /* trim the spec file line of leading and trailing whitespace */
                    specentry->data = strtrim(specentry->data);
                    assert(specentry->data != NULL);

                    /* nothing from the changelog on */
                    if (strprefix(specentry->data, SPEC_SECTION_CHANGELOG)) {
                        break;
                    }

                    /* read patch lines */
                    if (strprefix(specentry->data, SPEC_TAG_PATCH) && strstr(specentry->data, ":")) {
                        /* split the line - first field is PatchN:, second is the patch */
                        fields = strsplit(specentry->data, ": \t");
                        len = list_len(fields);

                        if (len < 2) {
                            errx(RI_PROGRAM_ERROR, "*** unable to parse line `%s'\n", specentry->data);
                        }

                        patch = TAILQ_FIRST(fields);
                        assert(patch != NULL);
                        entry = TAILQ_NEXT(patch, items);
                        assert(entry != NULL);

                        /* see if we have this patch */
                        if (!list_contains(patchfiles, entry->data)) {
                            list_free(fields, free);
                            continue;
                        }

                        /* extract just the number from the tag */
                        buf = patch->data;
                        buf += strlen(SPEC_TAG_PATCH);

                        /* patch entry may have leading whitespace */
                        while (isspace(*(entry->data)) && *(entry->data) != '\0') {
                            entry->data++;
                        }

                        /* add a new patch entry to the hash table */
                        hentry = calloc(1, sizeof(*hentry));
                        assert(hentry != NULL);
                        hentry->patch = strdup(entry->data);
                        hentry->num = strtoll(buf, NULL, 10);

                        if (errno == ERANGE || errno == EINVAL) {
                            warn("strtoll");
                            hentry->num = -1;
                        }

                        HASH_ADD_KEYPTR(hh, patches, hentry->patch, strlen(hentry->patch), hentry);
                    } else if (strprefix(specentry->data, SPEC_MACRO_PATCH)) {
                        /* split the line - first field is %patchN, second (optional) is opts */
                        fields = strsplit(specentry->data, " \t");
                        patch = TAILQ_FIRST(fields);
                        assert(patch != NULL);
                        entry = TAILQ_NEXT(patch, items);

                        /* extract just the number from the macro */
                        buf = patch->data;
                        len = strlen(buf);
                        buf += strlen(SPEC_MACRO_PATCH);
                        buf[strcspn(buf, " \t")] = '\0';

                        /* add a new patch entry to the hash table */
                        aentry = calloc(1, sizeof(*aentry));
                        assert(aentry != NULL);
                        aentry->num = strtoll(buf, NULL, 10);

                        if (errno == ERANGE || errno == EINVAL) {
                            warn("strtoll");
                            aentry->num = -1;
                        }

                        /* collect any options to the patch macro if present */
                        if (entry && entry->data && (strlen(specentry->data) > strlen(patch->data))) {
                            buf = specentry->data;
                            buf += len;                  /* advance past the first token */

                            while (isspace(*buf) && *buf != '\0') {
                                buf++;
                            }

                            aentry->opts = strdup(buf);
                        }

                        HASH_ADD_INT(applied, num, aentry);
                    }

                    /* clean up */
                    list_free(fields, free);
                }

                /* clean up the spec file we read in */
                list_free(speclines, free);
            }
        }

        list_free(patchfiles, free);

        /* Iterate over the SRPM files */
        TAILQ_FOREACH(file, peer->after_files, items) {
            if (!patches_driver(ri, file)) {
                result = !(params.severity >= RESULT_VERIFY);
            }
        }

        /* Report any removed patch files from the SRPM */
        if (peer->before_files) {
            TAILQ_FOREACH(file, peer->before_files, items) {
                if (file->peer_file == NULL) {
                    xasprintf(&params.msg, _("Patch file `%s` removed"), file->localpath);
                    add_result(ri, &params);
                    free(params.msg);
                    reported = true;
                    result = !(params.severity >= RESULT_VERIFY);
                }
            }
        }
    }

    /* Clean up the patches and applied hash tables */
    free_applied_patches(applied);
    free_patches(patches);

    /* Sound the everything-is-ok alarm if everything is, in fact, ok */
    if (result && !reported) {
        init_result_params(&params);
        params.header = NAME_PATCHES;
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        add_result(ri, &params);
    }

    return result;
}
