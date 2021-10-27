/*
 * Copyright © 2021 Red Hat, Inc.
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
#include <assert.h>
#include <libgen.h>
#include <errno.h>
#include <err.h>
#include <ftw.h>
#include <rpm/rpmspec.h>
#include <rpm/rpmbuild.h>
#include <rpm/rpmlog.h>
#include <unicode/ustdio.h>
#include <unicode/ustring.h>
#include "rpminspect.h"

/* subdirectories to create or link for the rpmbuild structure */
static char *subdirs[] = { RPMBUILD_BUILDDIR,
                           RPMBUILD_BUILDROOTDIR,
                           RPMBUILD_RPMDIR,
                           RPMBUILD_SOURCEDIR,
                           RPMBUILD_SPECDIR,
                           RPMBUILD_SRPMDIR,
                           NULL };

/* has the SRPM been checked? */
static bool seen = false;

/* these globals are used by the nftw() helper */
static char *build = NULL;
static struct rpminspect *globalri = NULL;
static bool globalresult = true;
static UChar32_list_t *forbidden = NULL;
static const char *globalarch = NULL;

/*
 * Given a spec file for a SRPM, do the equivalent of 'rpmbuild -bp'
 * to get an extracted and prepared source tree (e.g., patched).
 * Returns an allocated string containing the path to the rpmbuild
 * BUILD subdirectory.  The caller is responsible for freeing the
 * returned string.
 *
 * A NULL return value indicates a failure to prepare the source tree.
 */
static char *prep_source(struct rpminspect *ri, const rpmfile_entry_t *file)
{
    rpmSpec spec = NULL;
    char *shortname = NULL;
    char *fullpath = NULL;
    char *dst = NULL;
    char *macro = NULL;
    int i = 0;
    int mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    rpmts ts = NULL;
    BTA_t ba = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* use the already existing source subdirectory */
    fullpath = strdup(file->fullpath);
    assert(fullpath != NULL);
    shortname = dirname(fullpath);
    assert(shortname != NULL);

    /* create rpmbuild-like directory structure and set macros */
    xasprintf(&dst, "%s/%s", ri->worksubdir, RPMBUILD_TOPDIR);
    assert(dst != NULL);

    if (mkdirp(dst, mode) == -1) {
        free(dst);
        return NULL;
    }

    free(dst);

    for (i = 0; subdirs[i] != NULL; i++) {
        xasprintf(&dst, "%s/%s/%s", ri->worksubdir, RPMBUILD_TOPDIR, subdirs[i]);
        assert(dst != NULL);

        if (!strcmp(subdirs[i], RPMBUILD_SOURCEDIR) || !strcmp(subdirs[i], RPMBUILD_SPECDIR)) {
            /* symlinks SOURCES and SPECS to where the SRPM is already extracted */
            if (symlink(shortname, dst) == -1) {
                warn("symlink");
            }
        } else {
            if (mkdirp(dst, mode) == -1) {
                warn("mkdirp");
            }
        }

        free(dst);
    }

    free(fullpath);

    xasprintf(&macro, "_topdir %s/%s", ri->worksubdir, RPMBUILD_TOPDIR);
    assert(macro != NULL);
    (void) rpmDefineMacro(NULL, macro, 0);
    free(macro);

    /* read in the spec file */
    spec = rpmSpecParse(file->fullpath, RPMBUILD_PREP | RPMSPEC_ANYARCH, NULL);

    /* run through the %prep stage */
    ba = calloc(1, sizeof(*ba));
    assert(ba != NULL);
    ba->buildAmount |= RPMBUILD_PREP;

    ts = rpmtsCreate();
    (void) rpmtsSetRootDir(ts, ri->worksubdir);
    rpmtsSetFlags(ts, rpmtsFlags(ts) | RPMTRANS_FLAG_NOPLUGINS);

    rpmSetVerbosity(RPMLOG_WARNING);

    if (rpmSpecBuild(ts, spec, ba)) {
        build = NULL;
    } else {
        xasprintf(&build, "%s/%s/%s", ri->worksubdir, RPMBUILD_TOPDIR, RPMBUILD_BUILDDIR);
    }

    /* clean up */
    rpmtsFree(ts);
    free(ba);
    rpmSpecFree(spec);

    return build;
}

/*
 * Returns true if the UChar is what we consider a line ending.
 *
 * This function contains code adapted from this blog post about
 * Unicode with the ICU library:
 *
 * https://begriffs.com/posts/2019-05-23-unicode-icu.html
 */
static bool end_of_line(const UChar c)
{
    if ((c >= 0xA && c <= 0xD) || c == 0x85 || c == 0x2028 || c == 0x2029) {
        return true;
    }

    return false;
}

/*
 * nftw() helper used to validate each source file.
 *
 * This function contains code adapted from this blog post about
 * Unicode with the ICU library:
 *
 * https://begriffs.com/posts/2019-05-23-unicode-icu.html
 */
static int validate_file(const char *fpath, __attribute__((unused)) const struct stat *sb, int tflag, __attribute__((unused)) struct FTW *ftwbuf)
{
    char *type = NULL;
    const char *localpath = fpath;
    string_entry_t *sentry = NULL;
    UFILE *src = NULL;
    UChar c;
    UChar *line = NULL;
    UChar *line_new = NULL;
    size_t i = 0;
    size_t sz = BUFSIZ;
    size_t linenum = 0;
    UChar *needle = NULL;
    size_t colnum = 0;
    UChar32_entry_t *uentry = NULL;
    struct result_params params;

    assert(globalri != NULL);

    /* Only looking at regular files */
    if (tflag == FTW_D || tflag == FTW_DNR || tflag == FTW_DP || tflag == FTW_NS) {
        return 0;
    }

    /* check for exclusion by regular expression */
    if ((globalri->unicode_exclude != NULL) && (regexec(globalri->unicode_exclude, fpath, 0, NULL, 0) == 0)) {
        return 0;
    }

    type = mime_type(fpath);

    /* check for exclusion by MIME type */
    if (globalri->unicode_excluded_mime_types != NULL && !TAILQ_EMPTY(globalri->unicode_excluded_mime_types)) {
        TAILQ_FOREACH(sentry, globalri->unicode_excluded_mime_types, items) {
            if (!strcmp(type, sentry->data)) {
                free(type);
                return 0;
            }
        }
    }

    /* ignore any non-text files */
    if (!strprefix(type, "text/")) {
        free(type);
        return 0;
    }

    free(type);

    /* check for exclusion by ignore list */
    if (build && strprefix(localpath, build)) {
        /*
         * this is a file in the prepared source tree, so trim the
         * build sub dir and make the path strings look like this:
         *
         *     rpminspect-1.47.0/lib/magic.c
         */
        localpath += strlen(build);

        while (*localpath == '/' && *localpath != '\0') {
            localpath++;
        }

        if (localpath == NULL) {
            warnx(_("empty localpath on %s"), fpath);
            return 0;
        }
    }

    if (ignore_path(globalri, NAME_UNICODE, localpath, build)) {
        return 0;
    }

    /* initialize reporting results */
    init_result_params(&params);
    params.severity = RESULT_BAD;
    params.waiverauth = WAIVABLE_BY_SECURITY;
    params.header = NAME_UNICODE;
    params.arch = globalarch;
    params.file = localpath;
    params.noun = _("forbidden code point in ${FILE}");
    params.verb = VERB_FAILED;
    params.remedy = REMEDY_UNICODE;

    /* Read in the file as Unicode data */
    src = u_fopen(fpath, "r", NULL, NULL);

    if (src == NULL) {
        warn("u_fopen");
        return 0;
    }

    line = calloc(sz, sizeof(*line));
    assert(line != NULL);
    linenum = 1;

    while (!u_feof(src)) {
        /* read in one whole line of text from the file */
        for (i = 0; (line[i] = u_fgetc(src)) != U_EOF && !end_of_line(line[i]); i++) {
            /* increase the buffer size if necessary */
            if (i >= sz) {
                sz *= 2;
                errno = 0;
                line_new = realloc(line, sz * sizeof(*line));

                if (errno == ENOMEM) {
                    warn("realloc");
                    free(line);
                }

                line = line_new;

                if (line == NULL) {
                    return 0;
                }
            }
        }

        /* eat newline if terminated by a carriage return */
        if (line[i] == 0xD && (c = u_fgetc(src)) != 0xA) {
            u_fungetc(c, src);
        }

        /* this is either U_EOF or newline, trim it */
        line[i] = '\0';

        /* check this line for any prohibited characters */
        TAILQ_FOREACH(uentry, forbidden, items) {
            needle = u_strchr32(line, uentry->data);

            if (needle != NULL) {
                /* forbidden code point found */
                colnum = u_strlen(line) - u_strlen(needle);
                xasprintf(&params.msg, _("A forbidden code point was found in the %s source file on line %ld at column %ld."), localpath, linenum, colnum);
                add_result(globalri, &params);
                free(params.msg);
            }
        }

        /* advance the line counter */
        linenum++;
    }

    u_fclose(src);
    free(line);

    return 0;
}

static bool unicode_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    assert(ri != NULL);
    assert(file != NULL);
    assert(ri->workdir != NULL);

    /* skip binary packages */
    if (!headerIsSource(file->rpm_header)) {
        return true;
    }

    /* for reporting results */
    globalarch = get_rpm_header_arch(file->rpm_header);
    assert(globalarch != NULL);

    /* when the spec file is found, prepare the source tree and check each file there */
    if (strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
        /* for the spec file, examine each file in the prepared source tree */
        if (prep_source(ri, file) == NULL) {
            return false;
        }

        /* our tree dive result is saved in 'globalresult', -1 here is an internal error */
        if (nftw(build, validate_file, FOPEN_MAX, FTW_MOUNT|FTW_PHYS) == -1) {
            warn("nftw");
        }
    }

    /* check the individual file */
    (void) validate_file(file->fullpath, NULL, FTW_F, NULL);

    return globalresult;
}

/*
 * Main driver for the 'unicode' inspection.
 */
bool inspect_unicode(struct rpminspect *ri)
{
    bool result = true;
    UChar32_entry_t *entry = NULL;
    string_entry_t *sentry = NULL;
    struct result_params params;

    assert(ri != NULL);

    /* only run if there are forbidden code points */
    if (ri->unicode_forbidden_codepoints != NULL && !TAILQ_EMPTY(ri->unicode_forbidden_codepoints)) {
        /* convert code points to UChar values */
        forbidden = calloc(1, sizeof(*forbidden));
        assert(forbidden != NULL);
        TAILQ_INIT(forbidden);

        TAILQ_FOREACH(sentry, ri->unicode_forbidden_codepoints, items) {
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            errno = 0;
            entry->data = strtol(sentry->data, NULL, 16);

            if (errno == ERANGE || errno == EINVAL) {
                warn("strtol");
                free(entry);
                continue;
            }

            TAILQ_INSERT_TAIL(forbidden, entry, items);
        }

        /* so the nftw() helper can report results */
        globalri = ri;

        /* run the inspection */
        result = foreach_peer_file(ri, NAME_UNICODE, unicode_driver, false);

        /* free the forbidden list memory */
        while (!TAILQ_EMPTY(forbidden)) {
            entry = TAILQ_FIRST(forbidden);
            TAILQ_REMOVE(forbidden, entry, items);
            free(entry);
        }

        free(forbidden);
    }

    /* report */
    init_result_params(&params);
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_UNICODE;

    if (result) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    } else if (!seen) {
        params.severity = RESULT_INFO;
        xasprintf(&params.msg, _("The unicode inspection is only for source packages, skipping."));
        add_result(ri, &params);
        free(params.msg);

        /*
         * There's no reason to fail this test for an informational message.
         */
        result = true;
    }

    free(build);
    return result;
}
