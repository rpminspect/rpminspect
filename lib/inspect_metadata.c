/*
 * Copyright (C) 2019  Red Hat, Inc.
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

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <sys/queue.h>
#include "rpminspect.h"

/*
 * Validate the metadata tags in the RPM headers.
 */
static bool valid_peers(struct rpminspect *ri, const Header before_hdr, const Header after_hdr) {
    bool ret = true;
    bool valid_subdomain = false;
    string_entry_t *subdomain = NULL;
    char *after_vendor = NULL;
    char *after_buildhost = NULL;
    char *after_summary = NULL;
    char *after_description = NULL;
    char *after_nevra = NULL;
    char *msg = NULL;
    char *dump = NULL;

    assert(ri != NULL);

    after_nevra = headerGetAsString(after_hdr, RPMTAG_NEVRA);

    after_vendor = headerGetAsString(after_hdr, RPMTAG_VENDOR);
    if (after_vendor && strcmp(after_vendor, ri->vendor)) {
        xasprintf(&msg, "Package Vendor \"%s\" is not \"%s\" in %s", after_vendor, ri->vendor, after_nevra);

        add_result(&ri->results, RESULT_BAD, NOT_WAIVABLE, HEADER_METADATA, msg, NULL, REMEDY_VENDOR);
        ret = false;

        free(msg);
    }

    after_buildhost = headerGetAsString(after_hdr, RPMTAG_BUILDHOST);
    if (after_buildhost && ri->buildhost_subdomain != NULL) {
        valid_subdomain = false;

        TAILQ_FOREACH(subdomain, ri->buildhost_subdomain, items) {
            if (strsuffix(after_buildhost, subdomain->data)) {
                valid_subdomain = true;
                break;
            }
        }

        if (!valid_subdomain) {
            xasprintf(&msg, "Package Build Host \"%s\" is not within an expected build host subdomain in %s", after_buildhost, after_nevra);

            add_result(&ri->results, RESULT_BAD, NOT_WAIVABLE, HEADER_METADATA, msg, NULL, REMEDY_BUILDHOST);
            ret = false;

            free(msg);
        }
    }

    after_summary = headerGetAsString(after_hdr, RPMTAG_SUMMARY);
    if (after_summary && has_bad_word(after_summary, ri->badwords)) {
        xasprintf(&msg, "Package Summary contains unprofessional language in %s", after_nevra);
        xasprintf(&dump, "Summary: %s", after_summary);

        add_result(&ri->results, RESULT_BAD, NOT_WAIVABLE, HEADER_METADATA, msg, dump, REMEDY_BADWORDS);
        ret = false;

        free(msg);
        free(dump);
    }

    after_description = headerGetAsString(after_hdr, RPMTAG_DESCRIPTION);
    if (after_description && has_bad_word(after_description, ri->badwords)) {
        xasprintf(&msg, "Package Description contains unprofessional language in %s:", after_nevra);

        add_result(&ri->results, RESULT_BAD, NOT_WAIVABLE, HEADER_METADATA, msg, after_description, REMEDY_BADWORDS);
        ret = false;

        free(msg);
    }

    if (before_hdr != NULL) {
        char *before_vendor = headerGetAsString(before_hdr, RPMTAG_VENDOR);
        char *before_summary = headerGetAsString(before_hdr, RPMTAG_SUMMARY);
        char *before_description = headerGetAsString(before_hdr, RPMTAG_DESCRIPTION);
        after_nevra = headerGetAsString(after_hdr, RPMTAG_NAME);

        if (strcmp(before_vendor, after_vendor)) {
            xasprintf(&msg, "Package Vendor changed from \"%s\" to \"%s\" in %s", before_vendor, after_vendor, after_nevra);

            add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_METADATA, msg, NULL, NULL);
            ret = false;

            free(msg);
        }

        if (strcmp(before_summary, after_summary)) {
            xasprintf(&msg, "Package Summary change from \"%s\" to \"%s\" in %s", before_summary, after_summary, after_nevra);

            add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_METADATA, msg, NULL, NULL);
            ret = false;

            free(msg);
        }

        if (strcmp(before_description, after_description)) {
            xasprintf(&msg, "Package Description changed in %s", after_nevra);
            xasprintf(&dump, "from:\n\n%s\n\nto:\n\n%s", before_description, after_description);

            add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_METADATA, msg, dump, NULL);
            ret = false;

            free(msg);
            free(dump);
        }
    }

    return ret;
}

/*
 * Main driver for the 'metadata' inspection.
 */
bool inspect_metadata(struct rpminspect *ri) {
    bool ret = false;
    rpmpeer_entry_t *peer = NULL;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    /*
     * The metadata inspection looks at the RPM header information and
     * performs a few checks:
     *     - Make sure the Vendor is correct
     *     - Make sure the Buildhost is in the expected subdomain
     *     - Check for bad words in the Summary and Description
     *     - In cases of before and after builds, check for differences
     *       between the above RPM header values and report them.
     */

    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are caught by INSPECT_EMPTYRPM */
        if (peer->after_rpm == NULL) {
            continue;
        }

        if (!valid_peers(ri, peer->before_hdr, peer->after_hdr)) {
            ret = true;
        }
    }

    if (ret) {
        add_result(&ri->results, RESULT_OK, NOT_WAIVABLE, HEADER_METADATA, NULL, NULL, NULL);
    }

    return ret;
}
