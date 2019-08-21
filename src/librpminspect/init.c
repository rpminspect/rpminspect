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
#include <search.h>
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
    char *token = NULL;
    string_entry_t *entry = NULL;
    uint64_t tests = 0;
    int nkeys = 0;
    int len = 0;
    const char **keys = NULL;
    const char *val = NULL;
    ENTRY e;
    ENTRY *eptr;

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

    tmp = iniparser_getstring(cfg, "koji:download_ursine", NULL);
    if (tmp == NULL) {
        ri->kojiursine = NULL;
    } else {
        ri->kojiursine = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "koji:download_mbs", NULL);
    if (tmp == NULL) {
        ri->kojimbs = NULL;
    } else {
        ri->kojimbs = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "tests:badwords", NULL);
    if (tmp == NULL) {
        ri->badwords = NULL;
    } else {
        /* make a copy of the string for splitting */
        start = walk = strdup(tmp);

        /* split up the string of bad words and turn them in to a list */
        ri->badwords = calloc(1, sizeof(*(ri->badwords)));
        assert(ri->badwords != NULL);
        TAILQ_INIT(ri->badwords);

        while ((token = strsep(&walk, " \t")) != NULL) {
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);

            entry->data = strdup(token);
            assert(entry->data != NULL);

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
        /* make a copy of the string for splitting */
        start = walk = strdup(tmp);

        /* split up the string of subdomains and turn them in to a list */
        ri->buildhost_subdomain = calloc(1, sizeof(*(ri->buildhost_subdomain)));
        assert(ri->buildhost_subdomain != NULL);
        TAILQ_INIT(ri->buildhost_subdomain);

        while ((token = strsep(&walk, " \t")) != NULL) {
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);

            entry->data = strdup(token);
            assert(entry->data != NULL);

            TAILQ_INSERT_TAIL(ri->buildhost_subdomain, entry, items);
        }

        /* clean up */
        free(start);
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

    tmp = iniparser_getstring(cfg, "tests:desktop_entry_files_dir", NULL);
    if (tmp == NULL) {
        ri->desktop_entry_files_dir = strdup(DESKTOP_ENTRY_FILES_DIR);
    } else {
        ri->desktop_entry_files_dir = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "tests:desktop_file_validate", NULL);
    if (tmp == NULL) {
        ri->desktop_file_validate = strdup(DESKTOP_FILE_VALIDATE);
    } else {
        ri->desktop_file_validate = strdup(tmp);
    }

    /* if a jvm major versions exist, collect those in to a hash table */
    ri->jvm_table = NULL;
    ri->jvm_keys = NULL;
    tmp = "javabytecode";
    len = strlen(tmp);
    nkeys = iniparser_getsecnkeys(cfg, tmp);

    if (nkeys > 0) {
        keys = calloc(nkeys, sizeof(*keys));
        assert(keys != NULL);

        if (iniparser_getseckeys(cfg, tmp, keys) == NULL) {
            free(keys);
            keys = NULL;
            nkeys = 0;
        }
    }

    if (nkeys > 0) {
        ri->jvm_table = calloc(1, sizeof(*ri->jvm_table));
        assert(ri->jvm_table != NULL);

        if (hcreate_r(nkeys, ri->jvm_table) == 0) {
            free(ri->jvm_table);
            ri->jvm_table = NULL;
            free(keys);
            keys = NULL;
            nkeys = 0;
        } else {
           ri->jvm_keys = calloc(1, sizeof(*ri->jvm_keys));
           assert(ri->jvm_keys != NULL);
           TAILQ_INIT(ri->jvm_keys);
        }
    }

    while (nkeys > 0) {
        val = iniparser_getstring(cfg, keys[nkeys - 1], NULL);

        if (val == NULL) {
            nkeys--;
            continue;
        }

        /*
         * This grabs the key string, but advances past the block name.
         * The key is stored in the tailq for the ability to free it
         * later.  We have to dupe it because the iniparser data will
         * go away.
         */
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->data = strdup((char *) keys[nkeys - 1] + len + 1);
        TAILQ_INSERT_TAIL(ri->jvm_keys, entry, items);

        e.key = entry->data;
        e.data = strdup((char *) val);
        hsearch_r(e, ENTER, &eptr, ri->jvm_table);
        nkeys--;
    }

    /* the rest of the members */
    ri->before = NULL;
    ri->after = NULL;
    ri->buildtype = KOJI_BUILD_RPM;
    ri->peers = init_rpmpeer();
    ri->worksubdir = NULL;
    ri->tests = ~tests;
    ri->results = NULL;
    ri->product_release = NULL;
    ri->arches = NULL;

    /* Clean up */
    iniparser_freedict(cfg);

    return 0;
}
