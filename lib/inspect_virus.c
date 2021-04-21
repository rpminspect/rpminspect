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

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <dirent.h>
#include <sys/types.h>
#include <clamav.h>
#include "rpminspect.h"

static bool clamav_ready = false;
static struct cl_engine *engine = NULL;
static unsigned int sigs = 0;
static struct result_params params;

static bool virus_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    int r = 0;
#ifndef CL_SCAN_STDOPT
    struct cl_scan_options opts;
#endif
    const char *virus = NULL;

    /* only check regular files */
    if (!S_ISREG(file->st.st_mode)) {
        return true;
    }

    /* initialize clamav if we need to */
    if (!clamav_ready) {
        /* create clamav engine */
        engine = cl_engine_new();

        if (engine == NULL) {
            errx(RI_PROGRAM_ERROR, _("cl_engine_new() returned NULL, check clamav library"));
        }

        /* load clamav databases */
        r = cl_load(cl_retdbdir(), engine, &sigs, CL_DB_STDOPT);

        if (r != CL_SUCCESS) {
            cl_engine_free(engine);
            errx(RI_PROGRAM_ERROR, _("cl_load(): %s"), cl_strerror(r));
        }

        /* compile engine */
        r = cl_engine_compile(engine);

        if (r != CL_SUCCESS) {
            cl_engine_free(engine);
            errx(RI_PROGRAM_ERROR, _("cl_engine_compile(): %s"), cl_strerror(r));
        }

        /* remember to not do all this again */
        clamav_ready = true;
    }

#ifndef CL_SCAN_STDOPT
    /* set up the clamav scan options */
    memset(&opts, 0, sizeof(opts));
    opts.general = CL_SCAN_GENERAL_ALLMATCHES | CL_SCAN_GENERAL_COLLECT_METADATA;
    opts.parse = ~0;

    /* disable broken ELF detection */
    opts.parse &= ~CL_SCAN_HEURISTIC_BROKEN;

    /* disable max limit detection (filesize, etc) */
    opts.parse &= ~CL_SCAN_HEURISTIC_EXCEEDS_MAX;

    /* scan the file */
    r = cl_scanfile(file->fullpath, &virus, NULL, engine, &opts);
#else
    /* scan the file */
    r = cl_scanfile(file->fullpath, &virus, NULL, engine, CL_SCAN_STDOPT);
#endif

    if (r == CL_VIRUS) {
        params.arch = get_rpm_header_arch(file->rpm_header);
        params.file = file->localpath;
        params.remedy = REMEDY_VIRUS;
        xasprintf(&params.msg, _("Virus detected in %s in the %s package on %s: %s"), file->localpath, headerGetString(file->rpm_header, RPMTAG_NAME), params.arch, virus);
        add_result(ri, &params);
        free(params.msg);

        result = false;
    } else if (r != CL_CLEAN) {
        warnx(_("cl_scanfile(%s): %s"), file->localpath, cl_strerror(r));
    }

    return result;
}

bool inspect_virus(struct rpminspect *ri)
{
    char *dbver = NULL;
    const char *dbpath = NULL;
    DIR *d = NULL;
    struct dirent *de = NULL;
    char *cvdpath = NULL;
    struct cl_cvd *cvd = NULL;
    bool result = false;
    int r = 0;

    /* initialize clamav */
    r = cl_init(CL_INIT_DEFAULT);

    if (r != CL_SUCCESS) {
        warnx(_("cl_init(): %s"), cl_strerror(r));
        return false;
    }

    /* set up result parameters */
    init_result_params(&params);

    /* display version information about clamav */
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.header = HEADER_VIRUS;
    xasprintf(&params.msg, _("clamav version information"));

    dbpath = cl_retdbdir();
    assert(dbpath != NULL);

    xasprintf(&params.details, _("clamav version %s"), cl_retver());

    d = opendir(dbpath);

    if (d == NULL) {
        err(EXIT_FAILURE, "opendir()");
    }

    errno = 0;

    while ((de = readdir(d)) != NULL) {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..") || !strsuffix(de->d_name, ".cvd")) {
            continue;
        }

        xasprintf(&cvdpath, "%s/%s", dbpath, de->d_name);
        assert(cvdpath != NULL);
        cvd = cl_cvdhead(cvdpath);

        xasprintf(&dbver, _("%s version %u (%s)"), cvdpath, cvd->version, cvd->time);
        assert(dbver != NULL);

        params.details = strappend(params.details, "\n", dbver, NULL);
        assert(params.details != NULL);

        cl_cvdfree(cvd);
        free(cvdpath);
        free(dbver);
    }

    if (errno != 0) {
        err(EXIT_FAILURE, "readdir()");
    }

    if (closedir(d) == -1) {
        warn("closedir()");
    }

    add_result(ri, &params);
    free(params.msg);

    /* run the virus check on each file */
    params.severity = RESULT_BAD;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = HEADER_VIRUS;
    params.verb = VERB_FAILED;
    params.noun = _("virus or malware in ${FILE}");
    result = foreach_peer_file(ri, NAME_VIRUS, virus_driver, false);

    /* hope the result is always this */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_VIRUS;
        add_result(ri, &params);
    }

    /* clean up */
    if (engine) {
        cl_engine_free(engine);
    }

    return result;
}
