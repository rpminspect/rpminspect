/*
 * Copyright (C) 2018-2020  Red Hat, Inc.
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

/**
 * @file inspect_license.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019-2020
 * @brief 'license' inspection
 * @copyright LGPL-3.0-or-later
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <json.h>
#include "rpminspect.h"

/* Local globals */
static struct json_object *licdb = NULL;
static char *licdata = NULL;
static int liclen = 0;

/* Local helper functions */
static struct json_object *read_licensedb(const char *licensedb)
{
    int fd = 0;

    assert(licensedb != NULL);

    fd = open(licensedb, O_RDONLY);

    if (fd == -1) {
        fprintf(stderr, _("*** Unable to open license db %s: %s\n"), licensedb, strerror(errno));
        fflush(stderr);
        return NULL;
    }

    liclen = lseek(fd, 0, SEEK_END);
    licdata = mmap(NULL, liclen, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    return json_tokener_parse(licdata);
}

/*
 * Called by is_valid_license() to check each short license token.  It
 * will also try to do a whole match on the license tag string.
 */
static bool check_license_abbrev(const char *lic)
{
    const char *fedora_abbrev = NULL;
    const char *spdx_abbrev = NULL;
    bool approved = false;

    assert(lic != NULL);

    json_object_object_foreach(licdb, license_name, val) {
        /* first reset our variables */
        fedora_abbrev = NULL;
        spdx_abbrev = NULL;
        approved = false;

        if (strlen(license_name) == 0) {
            continue;
        }

        /* collect the properties */
        json_object_object_foreach(val, prop, propval) {
            if (!strcmp(prop, "fedora_abbrev")) {
                fedora_abbrev = json_object_get_string(propval);
            } else if (!strcmp(prop, "spdx_abbrev")) {
                spdx_abbrev = json_object_get_string(propval);
            } else if (!strcmp(prop, "approved") && !strcasecmp(json_object_get_string(propval), "yes")) {
                approved = true;
            }
        }

        /*
         * no full tag match and no abbreviations, license entry invalid
         */
        if (strlen(fedora_abbrev) == 0 && strlen(spdx_abbrev) == 0) {
            continue;
        }

        /*
         * if the entire license string is approved, that is valid
         * if we hit 'fedora_abbrev', that is valid
         * if we hit 'spdx_abbrev' and approved is true, that is valid
         * NOTE: we only match the first hit in the license database
         */
        if (approved && (!strcmp(lic, fedora_abbrev) || !strcmp(lic, spdx_abbrev))) {
            return true;
        }
    }

    return false;
}

/*
 * Dupe or append a token to the given string.
 */
static void token_append(char **dest, const char *token)
{
    char *newdest = NULL;

    assert(token != NULL);

    if (*dest == NULL) {
        *dest = strdup(token);
        assert(*dest != NULL);
    } else {
        xasprintf(&newdest, "%s %s", *dest, token);
        assert(newdest != NULL);
        free(*dest);
        *dest = newdest;
    }

    return;
}

/*
 * Given a license expression without parens, check it.
 */
static bool is_valid_expression(const char *s, struct result_params *params, results_t **rq)
{
    bool result = true;
    char *tagtokens = NULL;
    char *tagcopy = NULL;
    char *token = NULL;
    char *lic = NULL;

    assert(s != NULL);
    assert(params != NULL);

    /* see if the entire substring matches */
    if (check_license_abbrev(s)) {
        DEBUG_PRINT("APPROVED: |%s|\n", s);
        return true;
    }

    /* fall back on checking each individual token */
    tagtokens = tagcopy = strdup(s);

    while ((token = strsep(&tagtokens, " ")) != NULL) {
        if (!strcmp(token, "")) {
            continue;
        }

        if (!strcasecmp(token, "and") || !strcasecmp(token, "or")) {
            if (lic == NULL) {
                continue;
            }

            if (!check_license_abbrev(lic)) {
                result = false;

                xasprintf(&params->msg, "%s is not an approved license", lic);
                add_result_entry(rq, params);
                free(params->msg);
            } else {
                DEBUG_PRINT("APPROVED: |%s|\n", lic);
            }

            free(lic);
            lic = NULL;
        } else if (lic == NULL) {
            lic = strdup(token);
        } else {
            token_append(&lic, token);
        }
    }

    /* check final token */
    if (lic) {
        if (!check_license_abbrev(lic)) {
            result = false;

            xasprintf(&params->msg, "%s is not an approved license", lic);
            add_result_entry(rq, params);
            free(params->msg);
        } else {
            DEBUG_PRINT("APPROVED: |%s|\n", lic);
        }
    }

    /* cleanup */
    free(lic);
    free(tagcopy);

    return result;
}

/*
 * RPM License tags in the spec file permit parentheses to group licenses
 * together that need to be used together.  The License tag also permits
 * the use of boolean 'and' and 'or' keywords.  The only thing of note for
 * these expressions is that they do not permit negation since that does
 * not really make sense for the License tag.  If a license doesn't apply,
 * the RPM cannot ship that.
 *
 * The check involved here consists of multiple parts:
 * 1) Check to see that expressions in parentheses are balance.  For
 *    example, "(GPLv2+ and MIT) or LGPLv2+" is valid, but we want to
 *    make sure that "GPLv2+ and MIT) or (LGPLv2+" is invalid.
 * 2) Tokenize the license tag.
 * 3) Iterate over each token, skipping the 'and' and 'or' keywords, to
 *    match against the license database.
 * 4) The function returns true if all license tags are approved in the
 *    database.  Any single tag that is unapproved results in false.
 */
static bool is_valid_license(struct rpminspect *ri, struct result_params *params, const char *licensedb, const char *tag)
{
    bool result = true;
    int balance = 0;
    size_t i = 0;
    char *tagtokens = NULL;
    char *tagcopy = NULL;
    char *token = NULL;
    results_t *rq = NULL;
    results_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(params != NULL);
    assert(licensedb != NULL);
    assert(tag != NULL);

    /* Set up the result parameters */
    params->severity = RESULT_BAD;
    params->remedy = REMEDY_UNAPPROVED_LICENSE;

    /* check for matching parens */
    for (i = 0; i < strlen(tag); i++) {
        if (balance < 0) {
            return false;
        }

        if (tag[i] == '(') {
            balance++;
            continue;
        }

        if (tag[i] == ')') {
            balance--;
        }
    }

    if (balance != 0) {
        return false;
    }

    /* read in the approved license database */
    if (licdb == NULL) {
        licdb = read_licensedb(licensedb);

        if (licdb == NULL) {
            return false;
        }
    }

    /* First try to match the entire string */
    if (check_license_abbrev(tag)) {
        DEBUG_PRINT("APPROVED: |%s|\n", tag);
        return true;
    }

    /* tokenize the license tag and validate each license */
    tagtokens = tagcopy = strdup(tag);

    while ((token = strsep(&tagtokens, "()")) != NULL) {
        if (!strcmp(token, "")) {
            continue;
        }

        if (!is_valid_expression(token, params, &rq)) {
            result = false;
        }
    }

    free(tagcopy);

    /*
     * license tag is approved if number of seen tags equals
     * number of valid tags
     */
    if (result) {
        free_results(rq);
        return true;
    } else {
        if (ri->results == NULL) {
            ri->results = init_results();
        }

        if (rq && !TAILQ_EMPTY(rq)) {
            /*
             * make sure to set the worst result based on queued
             * license inspection failures.
             */
            TAILQ_FOREACH(entry, rq, items) {
                if (entry->severity > ri->worst_result) {
                    ri->worst_result = entry->severity;
                }
            }

            /* append the queued license inspection failures */
            TAILQ_CONCAT(ri->results, rq, items);
        }

        return false;
    }

    return false;
}

/*
 * Called by inspect_license()
 */
static int check_peer_license(struct rpminspect *ri, struct result_params *params, const char *actual_licensedb, const Header hdr)
{
    int ret = 0;
    bool valid = false;
    char *nevra = NULL;
    const char *license = NULL;

    assert(ri != NULL);
    assert(params != NULL);
    assert(actual_licensedb != NULL);

    nevra = get_nevra(hdr);
    assert(nevra != NULL);
    license = headerGetString(hdr, RPMTAG_LICENSE);

    if (license == NULL) {
        xasprintf(&params->msg, _("Empty License Tag in %s"), nevra);
        params->severity = RESULT_BAD;
        params->remedy = REMEDY_LICENSE;
        add_result(ri, params);
        free(params->msg);
        ret = 1;
    } else {
        /* is the license tag valid or not */
        valid = is_valid_license(ri, params, actual_licensedb, license);

        if (valid) {
            xasprintf(&params->msg, _("Valid License Tag in %s: %s"), nevra, license);
            params->severity = RESULT_INFO;
            params->remedy = NULL;
            add_result(ri, params);
            free(params->msg);
            ret = 1;
        }

        /* does the license tag contain bad words? */
        if (has_bad_word(license, ri->badwords)) {
            xasprintf(&params->msg, _("License Tag contains unprofessional language in %s: %s"), nevra, license);
            params->severity = RESULT_BAD;
            params->remedy = REMEDY_LICENSE;
            add_result(ri, params);
            free(params->msg);
            ret = 1;
        }
    }

    free(nevra);
    return ret;
}

/*
 * Helper function to clean up the static globals here.
 */
static void free_licensedb(void)
{
    int r;

    /* ignore unused variable warnings if assert is disabled */
    (void) r;

    if (licdb == NULL) {
        return;
    }

    json_object_put(licdb);
    r = munmap(licdata, liclen);
    assert(r == 0);
    licdata = NULL;
    liclen = 0;
    licdb = NULL;

    return;
}

/**
 * @brief Perform the 'license' inspection.
 *
 * Verify the string specified in the License tag of the RPM metadata
 * describes permissible software licenses as defined by the license
 * database. Also checks to see if the License tag contains any
 * unprofessional words as defined in the configuration file.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_license(struct rpminspect *ri)
{
    int good = 0;
    int seen = 0;
    bool result = false;
    rpmpeer_entry_t *peer = NULL;
    char *actual_licensedb = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    init_result_params(&params);
    params.header = HEADER_LICENSE;
    params.waiverauth = NOT_WAIVABLE;

    xasprintf(&actual_licensedb, "%s/%s/%s", ri->vendor_data_dir, LICENSES_DIR, ri->licensedb);

    if (ri->licensedb == NULL || access(actual_licensedb, F_OK|R_OK)) {
        xasprintf(&params.msg, _("Missing license database: %s: %s"), actual_licensedb, strerror(errno));
        params.severity = RESULT_BAD;
        params.remedy = REMEDY_LICENSEDB;
        add_result(ri, &params);
        free(params.msg);
        free(actual_licensedb);
        return false;
    }

    /*
     * The license test just looks at the licenses on the after build
     * packages.  The before build is not used here.
     */

    /* Second check the binary peers */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are reported via INSPECT_EMPTYRPM */
        if (peer->after_rpm == NULL) {
            continue;
        }

        good += check_peer_license(ri, &params, actual_licensedb, peer->after_hdr);
        seen++;
    }

    /* Clean up */
    free_licensedb();
    free(actual_licensedb);

    if (good == seen) {
        params.severity = RESULT_OK;
        params.msg = NULL;
        add_result(ri, &params);
        result = true;
    }

    return result;
}
