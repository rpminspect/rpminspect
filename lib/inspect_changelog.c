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

#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <rpm/header.h>

#include "rpminspect.h"

/*
 * Given an RPM header, read the %changelog and reconstruct it as
 * a string_list_t where each entry is a changelog entry.
 */
static string_list_t *get_changelog(const Header hdr)
{
    string_list_t *changelog = NULL;
    string_entry_t *entry = NULL;
    rpmtd times = NULL;
    rpmtd names = NULL;
    rpmtd lines = NULL;
    rpmFlags flags = HEADERGET_MINMEM | HEADERGET_EXT | HEADERGET_ARGV;
    time_t timestamp;
    struct tm *logtime = NULL;
    const char *name = NULL;
    const char *line = NULL;
    char *lbuf = NULL;
    char tbuf[16];

    assert(hdr != NULL);

    /* start the changelog */
    changelog = calloc(1, sizeof(*changelog));
    assert(changelog != NULL);
    TAILQ_INIT(changelog);

    /* Read this RPM header and construct a new changelog */
    times = rpmtdNew();
    names = rpmtdNew();
    lines = rpmtdNew();

    if (headerGet(hdr, RPMTAG_CHANGELOGTIME, times, flags) &&
        headerGet(hdr, RPMTAG_CHANGELOGNAME, names, flags) &&
        headerGet(hdr, RPMTAG_CHANGELOGTEXT, lines, flags)) {
        while ((rpmtdNext(times) != -1) &&
               (rpmtdNext(names) != -1) &&
               (rpmtdNext(lines) != -1)) {
            /* Get all the parts of the changelog entry */
            timestamp = rpmtdGetNumber(times);
            logtime = gmtime(&timestamp);
            name = rpmtdGetString(names);
            line = rpmtdGetString(lines);

            /* trim any line endings */
            if (line) {
                lbuf = strdup(line);
                lbuf[strcspn(lbuf, "\r\n")] = 0;
            } else {
                lbuf = strdup("");
            }

            /* Convert the time in to an RPM-like string */
            strftime(tbuf, sizeof(tbuf), "%a %b %d %Y", logtime);

            /* Create a new changelog entry */
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            xasprintf(&entry->data, "* %s %s\n%s\n", tbuf, name, lbuf);
            free(lbuf);

            /* Add the changelog entry to the list */
            TAILQ_INSERT_TAIL(changelog, entry, items);
        }
    }

    /* Cleanup */
    rpmtdFreeData(times);
    rpmtdFreeData(names);
    rpmtdFreeData(lines);
    rpmtdFree(times);
    rpmtdFree(names);
    rpmtdFree(lines);

    return changelog;
}

/*
 * Perform %changelog checks on the SRPM packages between builds. Do
 * the following:
 *     - Report if the %changelog was removed in the after build but
 *       was preset in the before build. (VERIFY)
 *     - Report if the %changelog was missing in the before build but
 *       added in the after build. (INFO)
 *     - Report if the first entries in the before and after changelogs
 *       are identical (report as "no new changelog entry"). (BAD)
 */
static bool check_src_rpm_changelog(struct rpminspect *ri, const rpmpeer_entry_t *peer)
{
    bool result = true;
    char *before_nevra = NULL;
    char *after_nevra = NULL;
    string_list_t *before_changelog = NULL;
    string_list_t *after_changelog = NULL;
    char *msg = NULL;
    string_entry_t *before = NULL;
    string_entry_t *after = NULL;
    char *clog = NULL;

    assert(ri != NULL);
    assert(peer != NULL);

    /* get the before and after changelogs */
    before_changelog = get_changelog(peer->before_hdr);
    after_changelog = get_changelog(peer->after_hdr);
    before = TAILQ_FIRST(before_changelog);
    after = TAILQ_FIRST(after_changelog);

    /* get reporting information */
    before_nevra = get_nevra(peer->before_hdr);
    after_nevra = get_nevra(peer->after_hdr);

    /* Perform checks */
    if ((before_changelog && !TAILQ_EMPTY(before_changelog)) && (after_changelog == NULL || TAILQ_EMPTY(after_changelog))) {
        msg = list_to_string(before_changelog);
        xasprintf(&clog, "%s build had this %%changelog:\n%s", before_nevra, msg);
        free(msg);

        xasprintf(&msg, _("%%changelog lost between the %s and %s builds"), before_nevra, after_nevra);
        add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_CHANGELOG, msg, clog, REMEDY_CHANGELOG);
        free(msg);
        free(clog);
        result = false;
    } else if ((before_changelog == NULL || TAILQ_EMPTY(before_changelog)) && (after_changelog && !TAILQ_EMPTY(after_changelog))) {
        msg = list_to_string(after_changelog);
        xasprintf(&clog, "%s build has this %%changelog:\n%s", after_nevra, msg);
        free(msg);

        xasprintf(&msg, _("Gained %%changelog between the %s and %s builds"), before_nevra, after_nevra);
        add_result(ri, RESULT_INFO, NOT_WAIVABLE, HEADER_CHANGELOG, msg, clog, REMEDY_CHANGELOG);
        free(msg);
        free(clog);
        result = false;
    } else if ((before_changelog == NULL || TAILQ_EMPTY(before_changelog)) && (after_changelog == NULL || TAILQ_EMPTY(after_changelog))) {
        xasprintf(&msg, _("No %%changelog present in the %s build"), after_nevra);
        add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_CHANGELOG, msg, NULL, REMEDY_CHANGELOG);
        free(msg);
        result = false;
    } else if (!strcmp(before->data, after->data)) {
        xasprintf(&msg, _("No new %%changelog entry in the %s build"), after_nevra);
        add_result(ri, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_CHANGELOG, msg, NULL, REMEDY_CHANGELOG);
        free(msg);
        result = false;
    }

    /* cleanup */
    list_free(before_changelog, free);
    list_free(after_changelog, free);
    free(before_nevra);
    free(after_nevra);

    return result;
}

/*
 * Perform %changelog checks on a single RPM packages between builds.
 * Do the following:
 *     - Report changed/removed lines as VERIFY
 *     - Report only added lines as INFO
 *     * Check for unprofessional language and report as VERIFY
 */
static bool check_bin_rpm_changelog(struct rpminspect *ri, const rpmpeer_entry_t *peer)
{
    bool result = true;
    char *before_nevra = NULL;
    char *after_nevra = NULL;
    string_list_t *before_changelog = NULL;
    string_list_t *after_changelog = NULL;
    char *bclog = NULL;
    char *aclog = NULL;
    char *msg = NULL;
    string_list_t *diffresult = NULL;
    char *difference = NULL;
    string_entry_t *entry = NULL;
    severity_t severity = RESULT_INFO;

    assert(ri != NULL);
    assert(peer != NULL);

    /* get reporting information */
    before_nevra = get_nevra(peer->before_hdr);
    after_nevra = get_nevra(peer->after_hdr);

    /* get the before and after changelogs */
    before_changelog = get_changelog(peer->before_hdr);
    bclog = list_to_string(before_changelog);
    after_changelog = get_changelog(peer->after_hdr);
    aclog = list_to_string(after_changelog);

    /* Compare the changelogs */
    diffresult = unified_str_diff(bclog, aclog);

    if (diffresult != NULL && !TAILQ_EMPTY(diffresult)) {
        /* Differences found, see what kind */
        TAILQ_FOREACH(entry, diffresult, items) {
            if (entry->data && entry->data[0] == '-') {
                severity = RESULT_VERIFY;
                break;
            }
        }

        difference = list_to_string(diffresult);
        list_free(diffresult, free);

        if (severity == RESULT_INFO) {
            xasprintf(&msg, _("%%changelog contains new text in the %s build"), after_nevra);
            add_result(ri, severity, NOT_WAIVABLE, HEADER_CHANGELOG, msg, difference, NULL);
            free(msg);
        } else if (severity == RESULT_VERIFY) {
            xasprintf(&msg, _("%%changelog modified between the %s and %s builds"), before_nevra, after_nevra);
            add_result(ri, severity, WAIVABLE_BY_ANYONE, HEADER_CHANGELOG, msg, difference, REMEDY_CHANGELOG);
            free(msg);
            result = false;
        }

        free(difference);
    }

    /* Check for bad words */
    TAILQ_FOREACH(entry, after_changelog, items) {
        if (has_bad_word(entry->data, ri->badwords)) {
            xasprintf(&msg, _("%%changelog entry has unprofessional language in the %s build"), after_nevra);
            add_result(ri, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_CHANGELOG, msg, entry->data, REMEDY_CHANGELOG);
            free(msg);
            result = false;
        }
    }

    /* cleanup */
    free(bclog);
    free(aclog);
    list_free(before_changelog, free);
    list_free(after_changelog, free);
    free(before_nevra);
    free(after_nevra);

    return result;
}

/*
 * Main driver for the 'changelog' inspection.
 */
bool inspect_changelog(struct rpminspect *ri)
{
    bool src_result = true;
    bool bin_result = true;
    rpmpeer_entry_t *peer = NULL;
    rpmpeer_entry_t *src = NULL;
    rpmpeer_entry_t *bin = NULL;

    assert(ri != NULL);

    /* skip this inspection on modules */
    if (ri->buildtype == KOJI_BUILD_MODULE) {
        return true;
    }

    /* Get the source and one binary package */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (src && bin) {
            break;
        }

        if (headerIsSource(peer->after_hdr)) {
            src = peer;
        } else {
            bin = peer;
        }
    }

    /* Check the packages */
    if (src) {
        src_result = check_src_rpm_changelog(ri, src);
    }

    if (bin) {
        bin_result = check_bin_rpm_changelog(ri, bin);
    }

    if (src_result && bin_result) {
        add_result(ri, RESULT_OK, NOT_WAIVABLE, HEADER_CHANGELOG, NULL, NULL, NULL);
        return true;
    } else {
        return false;
    }
}
