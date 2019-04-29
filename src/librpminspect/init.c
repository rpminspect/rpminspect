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

#include "config.h"

#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <iniparser.h>
#include "rpminspect.h"

static int _add_regex(dictionary *cfg, const char *key, regex_t **regex_out)
{
    const char *pattern;
    int reg_result;
    char *errbuf = NULL;
    size_t errbuf_size;

    assert(regex_out);

    pattern = iniparser_getstring(cfg, key, NULL);
    if ((pattern == NULL) || (*pattern == '\0')) {
        *regex_out = NULL;
        return 0;
    }

    *regex_out = calloc(1, sizeof(regex_t));
    assert(*regex_out != NULL);

    if ((reg_result = regcomp(*regex_out, pattern, REG_EXTENDED)) != 0) {
        /* Get the size needed for the error message */
        errbuf_size = regerror(reg_result, *regex_out, NULL, 0);
        errbuf = malloc(errbuf_size);
        assert(errbuf != NULL);

        regerror(reg_result, *regex_out, errbuf, errbuf_size);
        fprintf(stderr, "*** Unable to compile regular expression %s: %s\n", pattern, errbuf);

        /* Clean up and return error */
        free(errbuf);
        free(*regex_out);
        return -1;
    }

    return 0;
}

/*
 * Initialize a struct rpminspect.  Called by applications using
 * librpminspect before they began calling library functions.
 * Return 0 on success, -1 on failure.
 */
int init_rpminspect(struct rpminspect *ri, const char *cfgfile) {
    dictionary *cfg = NULL;
    const char *tmp = NULL;
    char *walk = NULL;
    char *start = NULL;
    char *badword = NULL;
    string_entry_t *entry = NULL;
    uint64_t tests = 0;

    assert(ri != NULL);
    memset(ri, 0, sizeof(*ri));

    /* Store full path to the config file */
    ri->cfgfile = realpath(cfgfile, NULL);

    /* In case we have a missing configuration file, defaults all the way */
    if ((ri->cfgfile == NULL) || (access(ri->cfgfile, F_OK|R_OK) == -1)) {
        free(ri->cfgfile);
        ri->cfgfile = NULL;

        ri->workdir = strdup(DEFAULT_WORKDIR);

        return 0;
    }

    /* Load the configuration file and get a dictionary */
    cfg = iniparser_load(ri->cfgfile);

    /* Read in settings from the config file, falling back on defaults */
    tmp = iniparser_getstring(cfg, "common:workdir", NULL);
    if (tmp == NULL) {
        ri->workdir = strdup(DEFAULT_WORKDIR);
    } else {
        ri->workdir = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "common:licensedb", NULL);
    if (tmp == NULL) {
        ri->licensedb = strdup(LICENSE_DB_FILE);
    } else {
        ri->licensedb = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "koji:hub", NULL);
    if (tmp == NULL) {
        ri->kojihub = NULL;
    } else {
        ri->kojihub = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "koji:download", NULL);
    if (tmp == NULL) {
        ri->kojidownload = NULL;
    } else {
        ri->kojidownload = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "tests:badwords", NULL);
    if (tmp == NULL) {
        ri->badwords = NULL;
    } else {
        /* make a copy of the string for splitting */
        start = walk = strdup(tmp);

        /* split up the string of bad words and turn them in to regexps */
        ri->badwords = calloc(1, sizeof(*(ri->badwords)));
        assert(ri->badwords != NULL);
        TAILQ_INIT(ri->badwords);

        /* convert each word to a regexp and add it to the list */
        while ((badword = strsep(&walk, " \t")) != NULL) {
            /* create the first pattern with a beginning word boundary */
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            xasprintf(&entry->data, "\\b%s", badword);
            TAILQ_INSERT_TAIL(ri->badwords, entry, items);

            /* create the second pattern with an ending word boundary */
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            xasprintf(&entry->data, "%s\\b", badword);
            TAILQ_INSERT_TAIL(ri->badwords, entry, items);
        }

        /* clean up */
        free(start);
    }

    tmp = iniparser_getstring(cfg, "tests:vendor", NULL);
    if (tmp == NULL) {
        ri->vendor = NULL;
    } else {
        ri->vendor = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "tests:buildhost_subdomain", NULL);
    if (tmp == NULL) {
        ri->buildhost_subdomain = NULL;
    } else {
        ri->buildhost_subdomain = strdup(tmp);
    }

    /* If any of the regular expressions fail to compile, stop and return failure */
    if (_add_regex(cfg, "tests:elf_path_include", &ri->elf_path_include) != 0) {
        return -1;
    }

    if (_add_regex(cfg, "tests:elf_path_exclude", &ri->elf_path_exclude) != 0) {
        return -1;
    }

    if (_add_regex(cfg, "tests:manpage_path_include", &ri->manpage_path_include) != 0) {
        return -1;
    }

    if (_add_regex(cfg, "tests:manpage_path_exclude", &ri->manpage_path_exclude) != 0) {
        return -1;
    }

    if (_add_regex(cfg, "tests:xml_path_include", &ri->xml_path_include) != 0) {
        return -1;
    }

    if (_add_regex(cfg, "tests:xml_path_exclude", &ri->xml_path_exclude) != 0) {
        return -1;
    }

    ri->before = NULL;
    ri->after = NULL;
    ri->before_srpm = NULL;
    ri->after_srpm = NULL;
    ri->peers = init_rpmpeer();
    ri->worksubdir = NULL;
    ri->tests = ~tests;
    ri->results = NULL;

    /* Clean up */
    iniparser_freedict(cfg);

    return 0;
}
