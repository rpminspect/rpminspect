/*
 * Copyright (C) 2018-2020  Red Hat, Inc.
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
static bool is_valid_license(struct rpminspect *ri, const char *licensedb, const char *tag)
{
    int tok_seen = 0;
    int paren_tok_seen = 0;
    int valid = 0;
    int paren_valid = 0;
    int balance = 0;
    size_t i = 0;
    char *tagtokens = NULL;
    char *tagcopy = NULL;
    char *token = NULL;
    char *lic = NULL;
    char *paren_lic = NULL;
    bool good_token = false;
    bool good_paren_token = false;
    bool collect = true;
    int paren = 0;
    char *msg = NULL;
    results_t *rq = NULL;
    results_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(licensedb != NULL);
    assert(tag != NULL);

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
        return true;
    }

    /* tokenize the license tag and validate each license */
    tagtokens = tagcopy = strdup(tag);
    DEBUG_PRINT("tag=|%s|\n", tag);

    while ((token = strsep(&tagtokens, " ")) != NULL) {
        /* skip over empty strings */
        if (strlen(token) == 0) {
            continue;
        }

        /* build up a license substring if we see parens */
        if (strprefix(token, "(")) {
            paren++;
        }

        if (strsuffix(token, ")")) {
            paren--;

            if (!paren) {
                collect = true;
            }
        }

        /* keep collecting the license string until we see a boolean */
        if (!strcasecmp(token, "and") || !strcasecmp(token, "or")) {
            collect = false;
        }

        /* Ignore leading and trailing parens */
        while (*token == '(') {
            token++;
        }

        if (strsuffix(token, ")")) {
            token[strcspn(token, ")")] = 0;
            token_append(&paren_lic, token);
        }

        DEBUG_PRINT("lic=|%s|, paren_lic=|%s|, paren=%d, token=|%s|\n", lic, paren_lic, paren, token);

        /* Abbreviated licenses may contain spaces, so rebuild it */
        if (collect) {
            token_append(&lic, token);
        }

        /* If inside parens, gather the boolean tokens too */
        if (paren) {
            token_append(&paren_lic, token);
        }

        if (collect) {
            continue;
        }

        /* iterate over the license database to match this license tag */
        if (lic) {
            good_token = check_license_abbrev(lic);
            tok_seen++;

            if (paren) {
                paren_tok_seen++;
            }

            DEBUG_PRINT("    lic=|%s|, tok_seen=%d, paren_tok_seen=%d\n", lic, tok_seen, paren_tok_seen);

            if (good_token) {
                valid++;
                DEBUG_PRINT("APPROVED lic=|%s|, paren_valid=%d, tok_seen=%d, valid=%d\n", lic, paren_valid, tok_seen, valid);
            } else {
                xasprintf(&msg, "`%s' is not an approved license", lic);
                add_result_entry(&rq, RESULT_BAD, NOT_WAIVABLE, HEADER_LICENSE, msg, NULL, REMEDY_UNAPPROVED_LICENSE);
                free(msg);
            }
        }

        if (!good_token && paren_lic) {
            good_paren_token = check_license_abbrev(paren_lic);

            if (good_paren_token) {
                DEBUG_PRINT("APPROVED paren_lic=|%s|, paren_valid=%d, tok_seen=%d, valid=%d\n", paren_lic, paren_valid, tok_seen, valid);
                tok_seen = tok_seen - paren_tok_seen;
                paren_valid++;
            } else {
                xasprintf(&msg, "`%s' is not an approved license", paren_lic);
                add_result_entry(&rq, RESULT_BAD, NOT_WAIVABLE, HEADER_LICENSE, msg, NULL, REMEDY_UNAPPROVED_LICENSE);
                free(msg);
            }
        }

        free(lic);
        lic = NULL;

        if (!paren) {
            free(paren_lic);
            paren_lic = NULL;
            paren_tok_seen = 0;
        }

        collect = true;
    }

    /* check the last license abbreviation */
    if (lic) {
        good_token = check_license_abbrev(lic);
        tok_seen++;

        if (paren) {
            paren_tok_seen++;
        }

        DEBUG_PRINT("    lic=|%s|, tok_seen=%d, paren_tok_seen=%d\n", lic, tok_seen, paren_tok_seen);

        if (good_token) {
            valid++;
            DEBUG_PRINT("APPROVED lic=|%s|, paren_valid=%d, tok_seen=%d, valid=%d\n", lic, paren_valid, tok_seen, valid);
        } else {
            xasprintf(&msg, "`%s' is not an approved license", lic);
            add_result_entry(&rq, RESULT_BAD, NOT_WAIVABLE, HEADER_LICENSE, msg, NULL, REMEDY_UNAPPROVED_LICENSE);
            free(msg);
        }
    }

    if (!good_token && paren_lic) {
        good_paren_token = check_license_abbrev(paren_lic);

        if (good_paren_token) {
            DEBUG_PRINT("APPROVED paren_lic=|%s|, paren_valid=%d, tok_seen=%d, valid=%d\n", paren_lic, paren_valid, tok_seen, valid);
            tok_seen = tok_seen - paren_tok_seen;
            paren_valid++;
        } else {
            xasprintf(&msg, "`%s' is not an approved license", paren_lic);
            add_result_entry(&rq, RESULT_BAD, NOT_WAIVABLE, HEADER_LICENSE, msg, NULL, REMEDY_UNAPPROVED_LICENSE);
            free(msg);
        }
    }

    /* cleanup */
    free(tagcopy);
    free(lic);

    /*
     * license tag is approved if number of seen tags equals
     * number of valid tags
     */
    DEBUG_PRINT("tok_seen=%d, paren=%d, paren_valid=%d, valid=%d\n", tok_seen, paren, paren_valid, valid);

    if (tok_seen == (valid + paren_valid)) {
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
}

/*
 * Called by inspect_license()
 */
static int check_peer_license(struct rpminspect *ri, const char *actual_licensedb, const Header hdr)
{
    int ret = 0;
    bool valid = false;
    char *nevra = NULL;
    const char *license = NULL;
    char *msg = NULL;

    assert(ri != NULL);
    assert(actual_licensedb != NULL);

    nevra = get_nevra(hdr);
    assert(nevra != NULL);
    license = headerGetString(hdr, RPMTAG_LICENSE);

    if (license == NULL) {
        xasprintf(&msg, _("Empty License Tag in %s"), nevra);
        add_result(ri, RESULT_BAD, NOT_WAIVABLE, HEADER_LICENSE, msg, NULL, REMEDY_LICENSE);
        ret = 1;
        free(msg);
    } else {
        /* is the license tag valid or not */
        valid = is_valid_license(ri, actual_licensedb, license);

        if (valid) {
            xasprintf(&msg, _("Valid License Tag in %s: %s"), nevra, license);
            add_result(ri, RESULT_INFO, NOT_WAIVABLE, HEADER_LICENSE, msg, NULL, NULL);
            free(msg);
            ret = 1;
        }

        /* does the license tag contain bad words? */
        if (has_bad_word(license, ri->badwords)) {
            xasprintf(&msg, _("License Tag contains unprofessional language in %s: %s"), nevra, license);
            add_result(ri, RESULT_BAD, NOT_WAIVABLE, HEADER_LICENSE, msg, NULL, REMEDY_LICENSE);
            free(msg);
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

/*
 * Main driver for the 'license' inspection.
 */
bool inspect_license(struct rpminspect *ri)
{
    int good = 0;
    int seen = 0;
    bool result = false;
    rpmpeer_entry_t *peer = NULL;
    char *msg = NULL;
    char *actual_licensedb = NULL;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    xasprintf(&actual_licensedb, "%s/%s/%s", ri->vendor_data_dir, LICENSES_DIR, ri->licensedb);

    if (ri->licensedb == NULL || access(actual_licensedb, F_OK|R_OK)) {
        xasprintf(&msg, _("Missing license database: %s: %s"), actual_licensedb, strerror(errno));
        add_result(ri, RESULT_BAD, NOT_WAIVABLE, HEADER_LICENSE, msg, NULL, REMEDY_LICENSEDB);
        free(msg);
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

        good += check_peer_license(ri, actual_licensedb, peer->after_hdr);
        seen++;
    }

    /* Clean up */
    free_licensedb();
    free(actual_licensedb);

    if (good == seen) {
        add_result(ri, RESULT_OK, NOT_WAIVABLE, HEADER_LICENSE, NULL, NULL, NULL);
        result = true;
    }

    return result;
}
