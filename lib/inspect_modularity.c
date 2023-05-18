/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <ftw.h>
#include <errno.h>
#include <err.h>
#include <assert.h>

#include "parser.h"
#include "rpminspect.h"

/* Globals */
static bool static_context = false;

/*
 * Called by nftw() to find and read /data/static_context from modulemd.txt
 */
static int read_modulemd(const char *fpath, __attribute__((unused)) const struct stat *sb, int tflag, __attribute__((unused)) struct FTW *ftwbuf)
{
    char *head = NULL;
    char *headptr = NULL;
    char *bn = NULL;
    char *sc_val = NULL;
    parser_plugin *p = &yaml_parser;
    parser_context *ctx = NULL;

    if (tflag != FTW_F) {
        return 0;
    }

    headptr = head = strdup(fpath);
    assert(headptr != NULL);
    bn = basename(head);

    /* try to read this file */
    if (strcmp(bn, MODULEMD_FILENAME)) {
        free(headptr);
        return 0;
    }

    if (p->parse_file(&ctx, fpath)) {
        warnx(_("ignoring malformed module metadata file: %s"), fpath);
        free(headptr);
        return -1;
    }

    sc_val = p->getstr(ctx, "data", "static_context");

    if (sc_val != NULL && !strcasecmp(sc_val, "true")) {
        static_context = true;
    }

    free(sc_val);
    p->fini(ctx);
    free(headptr);
    return 0;
}

/*
 * Given a directory and a build subdirectory, construct a new full
 * path and walk that tree looking for the modulemd.txt file and then
 * reading /data/static_context from it.
 */
static bool get_static_context(const char *subdir, const char *build)
{
    char *path = NULL;

    assert(subdir != NULL);
    assert(build != NULL);

    /* create the build path */
    path = joinpath(subdir, build, NULL);
    assert(path != NULL);

    /* find the modulemd.txt file and read /data/static_context */
    static_context = false;

    if (nftw(path, read_modulemd, FOPEN_MAX, FTW_PHYS) == -1) {
        warn("nftw");
        free(path);
        return false;
    }

    /* cleanup */
    free(path);

    return static_context;
}

/*
 * Read the /data/static_context value from the modulemd.txt files and
 * validate them against the rules.
 */
static bool check_static_context(struct rpminspect *ri)
{
    bool r = true;
    bool bsc = false;
    bool asc = false;
    struct result_params params;
    char *after_compliance = NULL;

    assert(ri != NULL);

    /* Set up the result parameters */
    init_result_params(&params);
    params.header = NAME_MODULARITY;
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;

    /* get the after build static_context first and check it's correct */
    asc = get_static_context(ri->worksubdir, AFTER_SUBDIR);

    if (asc && ri->modularity_static_context == STATIC_CONTEXT_FORBIDDEN) {
        xasprintf(&after_compliance, _("The /data/static_context value in %s is %s, but the product release rules forbid the presence of /data/static_context in the module metadata."), ri->after, asc ? "true" : "false");
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.remedy = REMEDY_MODULARITY_STATIC_CONTEXT;
        r = false;
    } else if (!asc && ri->modularity_static_context == STATIC_CONTEXT_REQUIRED) {
        xasprintf(&after_compliance, _("The /data/static_context value in %s is %s, but the product release rules require the presence of /data/static_context in the module metadata."), ri->after, asc ? "true" : "false");
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.remedy = REMEDY_MODULARITY_STATIC_CONTEXT;
        r = false;
    } else if (!asc && ri->modularity_static_context == STATIC_CONTEXT_RECOMMEND) {
        xasprintf(&after_compliance, _("The /data/static_context value in %s is %s, but the product release rules recommend the presence of /data/static_context in the module metadata."), ri->after, asc ? "true" : "false");
    } else if (ri->modularity_static_context == STATIC_CONTEXT_NULL) {
        xasprintf(&after_compliance, _("The /data/static_context value in %s is %s, and the product release rules do not have a setting for /data/static_context in the module metadata."), ri->after, asc ? "true" : "false");
    } else {
        xasprintf(&after_compliance, _("The /data/static_context value in %s is %s as required by the product release rules."), ri->after, asc ? "true" : "false");
    }

    if (ri->before) {
        /* compare builds, report the changes */
        bsc = get_static_context(ri->worksubdir, BEFORE_SUBDIR);

        if (bsc == asc) {
            xasprintf(&params.msg, _("%s The /data/static_context value is the same as in %s."), after_compliance, ri->before);
        } else {
            xasprintf(&params.msg, _("%s The /data/static_context value has changed since %s."), after_compliance, ri->before);
        }
        free(after_compliance);
    } else {
        params.msg = after_compliance;
    }

    /* report */
    if (params.msg) {
        add_result(ri, &params);
        free(params.msg);
    }

    return r;
}

static bool modularity_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    rpmTagType tt;
    rpmTagVal tv;
    const char *modularitylabel = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Set up the result parameters */
    init_result_params(&params);
    params.severity = RESULT_BAD;
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_MODULARITY;
    params.remedy = REMEDY_MODULARITY;

    /* Build the message we'll use for errors */
    xasprintf(&params.msg, _("Package \"%s\" is part of a module but lacks the '%%{modularitylabel}' header tag."), headerGetString(file->rpm_header, RPMTAG_NAME));

    /* Find how to find the header */
    tv = rpmTagGetValue("modularitylabel");
    if (tv == RPMTAG_NOT_FOUND) {
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    tt = rpmTagGetTagType(tv);
    if (tt == RPM_NULL_TYPE) {
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    /* Get the tag from the header */
    modularitylabel = headerGetString(file->rpm_header, tv);

    if (modularitylabel == NULL) {
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    free(params.msg);
    return result;
}

/*
 * Main driver for the 'modularity' inspection.
 */
bool inspect_modularity(struct rpminspect *ri)
{
    bool tag_result = false;
    bool static_context_result = false;
    bool result = false;
    struct result_params params;

    assert(ri != NULL);

    if (ri->buildtype != KOJI_BUILD_MODULE) {
        init_result_params(&params);
        xasprintf(&params.msg, _("Inspection skipped because this build's type is not `module'."));
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_MODULARITY;
        add_result(ri, &params);
        free(params.msg);
        return true;
    }

    /* check each RPM for the modularitylabel tag */
    tag_result = foreach_peer_file(ri, NAME_MODULARITY, modularity_driver);

    /* check static context against static context rule */
    static_context_result = check_static_context(ri);

    /* final check of all results */
    result = tag_result && static_context_result;

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_MODULARITY;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
