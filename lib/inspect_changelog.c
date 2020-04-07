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

            /* Convert the time in to an RPM-like string */
            strftime(tbuf, sizeof(tbuf), "%a %b %d %Y", logtime);

            /* Create a new changelog entry */
            /*
             * The entry reconstruction comes from rpmpopt.in in the rpm
             * source:
             *     rpm alias --changes --qf '[* %{CHANGELOGTIME:date} %{CHANGELOGNAME}\n%{CHANGELOGTEXT}\n\n]' \
             *         --POPTdesc=$"list changes for this package with full time stamps"
             * Which is worth noting here because when you query the changelog
             * from an RPM (rpm -qp --changelog), it is reproducing the
             * %changelog section from the spec file entry by entry and the
             * actual number of blank lines may not be the same.
             */
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            xasprintf(&entry->data, "* %s %s\n%s\n\n", tbuf, name, line);

            DEBUG_PRINT("\n%s\n", entry->data);

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
 * Generate temporary changelog files for use with diff(1).
 */
static char *create_changelog(const string_list_t *changelog, const char *where)
{
    int fd;
    char *output = NULL;
    string_entry_t *entry = NULL;
    FILE *logfp = NULL;

    assert(changelog != NULL);
    assert(where != NULL);

    xasprintf(&output, "%s/changelog.XXXXXX", where)
    fd = mkstemp(output);

    if (fd == -1) {
        fprintf(stderr, "*** unable to create temporary file %s: %s\n", output, strerror(errno));
        return NULL;
    }

    logfp = fdopen(fd, "w");

    if (logfp == NULL) {
        fprintf(stderr, "*** unable to open temporary file %s for writing: %s\n", output, strerror(errno));
        close(fd);
        return NULL;
    }

    TAILQ_FOREACH(entry, changelog, items) {
        fprintf(logfp, "%s", entry->data);
    }

    if (fclose(logfp) != 0) {
        fprintf(stderr, "*** unable to close writing to temporary file %s: %s\n", output, strerror(errno));
        close(fd);
        return NULL;
    }

    return output;
}

/*
 * Given 'diff -u' output, advance the string past the headers.
 */
static char *skip_diff_headers(char *diff_output)
{
    char *diff_walk = diff_output;

    if (diff_output == NULL) {
        return diff_output;
    }

    if (!strncmp(diff_walk, "--- ", 4)) {
        diff_walk = strchr(diff_walk, '\n') + 1;

        while (*diff_walk == '\n') {
            diff_walk++;
        }
    }

    if (!strncmp(diff_walk, "+++ ", 4)) {
        diff_walk = strchr(diff_walk, '\n') + 1;

        while (*diff_walk == '\n') {
            diff_walk++;
        }
    }

    return diff_walk;
}

/*
 * Analyze the 'diff -u' output and determine the severity of the result.
 */
static severity_t get_diff_severity(const char *diff_output)
{
    char *diff_walk = NULL;
    char *line = NULL;
    long int add = 0;
    long int del = 0;
    severity_t sev = RESULT_OK;

    if (diff_output == NULL) {
        return RESULT_INFO;
    }

    diff_walk = strdup(diff_output);
    assert(diff_walk != NULL);

    while ((line = strsep(&diff_walk, "\n")) != NULL) {
        DEBUG_PRINT("line=|%s|\n", line);

        if (strlen(line) >= 2 && line[1] == ' ') {
            if (line[0] == '+') {
                add++;
            } else if (line[0] == '-') {
                del++;
            }
        }
    }

    if (del > add) {
        sev = RESULT_VERIFY;
    } else {
        sev = RESULT_INFO;
    }

    free(diff_walk);
    return sev;
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
    char *before_output = NULL;
    char *after_output = NULL;
    char *diff_output = NULL;
    char *full_diff_output = NULL;
    int exitcode = 0;
    char *diff_walk = NULL;

    assert(ri != NULL);
    assert(peer != NULL);

    /* get reporting information */
    before_nevra = get_nevra(peer->before_hdr);
    after_nevra = get_nevra(peer->after_hdr);

    /* get the before and after changelogs */
    before_changelog = get_changelog(peer->before_hdr);
    after_changelog = get_changelog(peer->after_hdr);
    before = TAILQ_FIRST(before_changelog);
    after = TAILQ_FIRST(after_changelog);

    /* Generate temporary changelog files */
    before_output = create_changelog(before_changelog, ri->workdir);
    after_output = create_changelog(after_changelog, ri->workdir);

    /* Compare the changelogs */
    diff_output = run_cmd(&exitcode, DIFF_CMD, "-u", before_output, after_output, NULL);

    if (exitcode) {
        /* Skip past the diff(1) header lines */
        diff_walk = skip_diff_headers(diff_output);
        full_diff_output = strdup(diff_walk);

        /* Perform checks */
        if (before_changelog && (after_changelog == NULL || TAILQ_EMPTY(after_changelog))) {
            xasprintf(&msg, "%%changelog lost between the %s and %s builds", before_nevra, after_nevra);
            add_result(ri, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_CHANGELOG, msg, full_diff_output, REMEDY_CHANGELOG);
            free(msg);
            result = false;
        } else if ((before_changelog == NULL || TAILQ_EMPTY(before_changelog)) && after_changelog) {
            xasprintf(&msg, "Gained %%changelog between the %s and %s builds", before_nevra, after_nevra);
            add_result(ri, RESULT_INFO, NOT_WAIVABLE, HEADER_CHANGELOG, msg, full_diff_output, NULL);
            free(msg);
            result = false;
        } else if ((before_changelog == NULL || TAILQ_EMPTY(before_changelog)) && (after_changelog == NULL || TAILQ_EMPTY(after_changelog))) {
            xasprintf(&msg, "No %%changelog present in the %s build", after_nevra);
            add_result(ri, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_CHANGELOG, msg, NULL, REMEDY_CHANGELOG);
            free(msg);
            result = false;
        } else if (!strcmp(before->data, after->data)) {
            xasprintf(&msg, "No new %%changelog entry in the %s build", after_nevra);
            add_result(ri, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_CHANGELOG, msg, full_diff_output, REMEDY_CHANGELOG);
            free(msg);
            result = false;
        }
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
    char *msg = NULL;
    char *before_output = NULL;
    char *after_output = NULL;
    char *diff_output = NULL;
    char *full_diff_output = NULL;
    int exitcode = 0;
    char *diff_walk = NULL;
    string_entry_t *entry = NULL;
    severity_t severity = RESULT_INFO;

    assert(ri != NULL);
    assert(peer != NULL);

    /* get reporting information */
    before_nevra = get_nevra(peer->before_hdr);
    after_nevra = get_nevra(peer->after_hdr);

    /* get the before and after changelogs */
    before_changelog = get_changelog(peer->before_hdr);
    after_changelog = get_changelog(peer->after_hdr);

    /* Generate temporary changelog files */
    before_output = create_changelog(before_changelog, ri->workdir);
    after_output = create_changelog(after_changelog, ri->workdir);

    /* Compare the changelogs */
    diff_output = run_cmd(&exitcode, DIFF_CMD, "-u", before_output, after_output, NULL);

    if (exitcode) {
        /* Skip past the diff(1) header lines */
        diff_walk = skip_diff_headers(diff_output);
        full_diff_output = strdup(diff_walk);

        /* Differences found, see what kind */
        severity = get_diff_severity(full_diff_output);

        if (severity == RESULT_INFO) {
            xasprintf(&msg, "%%changelog contains new text in the %s build", after_nevra);
            add_result(ri, severity, NOT_WAIVABLE, HEADER_CHANGELOG, msg, full_diff_output, NULL);
            free(msg);
        } else if (severity == RESULT_VERIFY) {
            xasprintf(&msg, "%%changelog modified between the %s and %s builds", before_nevra, after_nevra);
            add_result(ri, severity, WAIVABLE_BY_ANYONE, HEADER_CHANGELOG, msg, full_diff_output, REMEDY_CHANGELOG);
            free(msg);
            result = false;
        }
    }

    free(diff_output);
    free(full_diff_output);
    free(before_output);
    free(after_output);

    /* Check for bad words */
    TAILQ_FOREACH(entry, after_changelog, items) {
        if (has_bad_word(entry->data, ri->badwords)) {
            xasprintf(&msg, "%%changelog entry has unprofessional language in the %s build", after_nevra);
            add_result(ri, RESULT_BAD, WAIVABLE_BY_ANYONE, HEADER_CHANGELOG, msg, entry->data, REMEDY_CHANGELOG);
            free(msg);
            result = false;
        }
    }

    /* cleanup */
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

    assert(src != NULL);
    assert(bin != NULL);

    /* Check the packages */
    src_result = check_src_rpm_changelog(ri, src);
    bin_result = check_bin_rpm_changelog(ri, bin);

    if (src_result && bin_result) {
        add_result(ri, RESULT_OK, NOT_WAIVABLE, HEADER_CHANGELOG, NULL, NULL, NULL);
        return true;
    } else {
        return false;
    }
}
