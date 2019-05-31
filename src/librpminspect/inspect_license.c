/*
 * Copyright (C) 2018-2019  Red Hat, Inc.
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

#include "config.h"

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
static struct json_object *_read_licensedb(const char *licensedb) {
    int fd = 0;

    assert(licensedb != NULL);

    fd = open(licensedb, O_RDONLY);

    if (fd == -1) {
        fprintf(stderr, "*** Unable to open license db %s: %s\n", licensedb, strerror(errno));
        fflush(stderr);
        return NULL;
    }

    liclen = lseek(fd, 0, SEEK_END);
    licdata = mmap(NULL, liclen, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    return json_tokener_parse(licdata);
}

/*
 * Called by inspect_license()
 */
static int _check_peer_license(struct rpminspect *ri, const Header hdr) {
    int ret = 0;
    bool valid = false;
    char *nevra = NULL;
    char *license = NULL;
    char *msg = NULL;

    assert(ri != NULL);
    assert(ri->licensedb != NULL);

    nevra = headerGetAsString(hdr, RPMTAG_NEVRA);
    license = headerGetAsString(hdr, RPMTAG_LICENSE);

    if (license == NULL) {
        xasprintf(&msg, "Empty License Tag in %s", nevra);
    } else {
        /* is the license tag valid or not */
        valid = is_valid_license(ri->licensedb, license);

        if (valid) {
            xasprintf(&msg, "Valid License Tag in %s: %s", nevra, license);

            add_result(&ri->results, RESULT_INFO, NOT_WAIVABLE, HEADER_LICENSE, msg, NULL, NULL);
            ret = 1;

            free(msg);
        } else {
            xasprintf(&msg, "Invalid License Tag in %s: %s", nevra, license);
        }

        /* does the license tag contain bad words? */
        if (has_bad_word(license, ri->badwords)) {
            xasprintf(&msg, "License Tag contains unprofessional language in %s: %s", nevra, license);
        }
    }

    if (msg != NULL && ret == 0) {
        add_result(&ri->results, RESULT_BAD, NOT_WAIVABLE, HEADER_LICENSE, msg, NULL, REMEDY_LICENSE);
        free(msg);
    }

    return ret;
}

/*
 * Helper function to clean up the static globals here.
 */
void free_licensedb(void) {
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
bool is_valid_license(const char *licensedb, const char *tag) {
    int seen = 0;
    int valid = 0;
    int balance = 0;
    size_t i = 0;
    char *tagtokens = NULL;
    char *tagcopy = NULL;
    char *lic = NULL;
    const char *fedora_abbrev = NULL;
    const char *spdx_abbrev = NULL;
    bool approved = false;
    bool wholetag = false;

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
        licdb = _read_licensedb(licensedb);

        if (licdb == NULL) {
            return false;
        }
    }

    /* tokenize the license tag and validate each license */
    tagtokens = tagcopy = strdup(tag);

    while ((lic = strsep(&tagtokens, "() ")) != NULL) {
        /* skip over empty strings and the boolean keywords */
        if (strlen(lic) == 0 || !strcasecmp(lic, "and") || !strcasecmp(lic, "or")) {
            continue;
        }

        wholetag = false;
        seen++;

        /* iterate over the license database to match this license tag */
        json_object_object_foreach(licdb, license_name, val) {
            /* first reset our variables */
            fedora_abbrev = NULL;
            spdx_abbrev = NULL;
            approved = false;

            /* if the entire license string matches the name, approved */
            if (!strcmp(tag, license_name)) {
                wholetag = true;
                break;
            }

            /* collect the properties */
            json_object_object_foreach(val, prop, propval) {
                if (!strcmp(prop, "fedora_abbrev")) {
                    fedora_abbrev = json_object_get_string(propval);
                } else if (!strcmp(prop, "spdx_abbrev")) {
                    spdx_abbrev = json_object_get_string(propval);
                } else if (!strcmp(prop, "approved")) {
                    approved = json_object_get_boolean(propval);
                }
            }

            /*
             * if either of these are empty, it means the entire license
             * name is the only thing we can match and if we got here it
             * means we didn't match it, so continue.
             */
            if (strlen(fedora_abbrev) == 0 && strlen(spdx_abbrev) == 0) {
                continue;
            }

            /*
             * if we hit 'fedora_abbrev', that is valid
             * if we hit 'spdx_abbrev' and approved is true, that is valid
             * NOTE: we only match the first hit in the license database
             */
            if (!strcmp(lic, fedora_abbrev) || (!strcmp(lic, spdx_abbrev) && approved)) {
                valid++;
                break;
            }
        }

        if (wholetag) {
            break;
        }
    }

    /* cleanup */
    free(tagcopy);

    /*
     * license tag is approved if number of seen tags equals
     * number of valid tags
     */
    return (seen == valid);
}

/*
 * Main driver for the 'license' inspection.
 */
bool inspect_license(struct rpminspect *ri) {
    int good = 0;
    int seen = 0;
    bool result;
    rpmpeer_entry_t *peer = NULL;

    assert(ri != NULL);
    assert(ri->peers != NULL);

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

        good += _check_peer_license(ri, peer->after_hdr);
        seen++;
    }

    /* Clean up */
    free_licensedb();

    result = (good == seen);

    if (result) {
        add_result(&ri->results, RESULT_OK, NOT_WAIVABLE, HEADER_LICENSE, NULL, NULL, NULL);
    }

    return result;
}
