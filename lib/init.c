/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
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

#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <search.h>
#include <iniparser.h>
#include "rpminspect.h"

static int add_regex(dictionary *cfg, const char *key, regex_t **regex_out)
{
    const char *pattern;
    int reg_result;
    char *errbuf = NULL;
    size_t errbuf_size;

    assert(regex_out);

    pattern = iniparser_getstring(cfg, key, NULL);
    if ((pattern == NULL) || (*pattern == '\0')) {
        return 0;
    }

    free_regex(*regex_out);
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

static void parse_list(const char *tmp, string_list_t **list)
{
    char *walk = NULL;
    char *start = NULL;
    char *token = NULL;
    string_entry_t *entry = NULL;

    assert(tmp != NULL);

    /* make a copy of the string for splitting */
    start = walk = strdup(tmp);

    /* split up the string and turn it in to a list */
    list_free(*list, free);
    *list = calloc(1, sizeof(*(*list)));
    assert(*list != NULL);
    TAILQ_INIT(*list);

    while ((token = strsep(&walk, " \t")) != NULL) {
        if (*token == '\0') {
            continue;
        }

        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);

        entry->data = strdup(token);
        assert(entry->data != NULL);

        TAILQ_INSERT_TAIL(*list, entry, items);
    }

    /* clean up */
    free(start);
    return;
}

/*
 * Read in a hash table mapping from the config file.  Example:
 *        [section]
 *        key = value
 *        key = value
 * We use these for the jvm_table and products, maybe other things.
 */
static void read_mapping(const dictionary *cfg, const char *section,
                         struct hsearch_data **table, string_list_t **keys)
{
    size_t len = 0;
    int nk = 0;
    const char **k = NULL;
    const char *v = NULL;
    ENTRY e;
    ENTRY *eptr;
    string_entry_t *entry = NULL;

    assert(cfg != NULL);
    assert(section != NULL);

    len = strlen(section);
    nk = iniparser_getsecnkeys(cfg, section);

    if (nk > 0) {
        k = calloc(nk, len);
        assert(k != NULL);

        if (iniparser_getseckeys(cfg, section, k) == NULL) {
            free(k);
            k = NULL;
            nk = 0;
        }
    }

    if (nk > 0) {
        free_mapping(*table, *keys);

        *table = calloc(1, sizeof(**table));
        assert(*table != NULL);

        if (hcreate_r(nk, *table) == 0) {
            free(*table);
            *table = NULL;
            free(k);
            k = NULL;
            nk = 0;
        } else {
            *keys = calloc(1, sizeof(**keys));
            assert(*keys != NULL);
            TAILQ_INIT(*keys);
        }
    }

    while (nk > 0) {
        v = iniparser_getstring(cfg, k[nk - 1], NULL);

        if (v == NULL) {
            nk--;
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
        entry->data = strdup((char *) k[nk - 1] + len + 1);
        TAILQ_INSERT_TAIL(*keys, entry, items);

        e.key = entry->data;
        e.data = strdup((char *) v);
        hsearch_r(e, ENTER, &eptr, *table);
        nk--;
    }

    free(k);
    return;
}

/*
 * Convert a 10 character mode string for a file to a mode_t
 * For example, convert "-rwsr-xr-x" to a mode_t
 */
static mode_t parse_mode(const char *input) {
    mode_t mode = 0;
    char i;

    assert(input != NULL);

    if (strlen(input) != 10) {
        fprintf(stderr, "*** Invalid input string `%s`\n", input);
        return mode;
    }

        /* type */
    i = input[0];
    if (i == 'd') {
        mode |= S_IFDIR;
    } else if (i == 'c') {
        mode |= S_IFCHR;
    } else if (i == 'b') {
        mode |= S_IFBLK;
    } else if (i == '-') {
        mode |= S_IFREG;
    } else if (i == 'l') {
        mode |= S_IFLNK;
    } else if (i == 's') {
        mode |= S_IFSOCK;
#ifdef S_IFIFO
    } else if (i == 'p') {
        mode |= S_IFIFO;
#endif
#ifdef S_IFWHT
    } else if (i == 'w') {
        mode |= S_IFWHT;
#endif
    } else if (i != '-') {
        fprintf(stderr, "*** Invalid mode string: %s\n", input);
        return mode;
    }

    /* owner */
    i = input[1];
    if (i == 'r') {
        mode |= S_IRUSR;
    } else if (i != '-') {
        fprintf(stderr, "*** Invalid mode string: %s\n", input);
        return mode;
    }

    i = input[2];
    if (i == 'w') {
        mode |= S_IWUSR;
    } else if (i != '-') {
        fprintf(stderr, "*** Invalid mode string: %s\n", input);
        return mode;
    }

    i = input[3];
    if (i == 'x') {
        mode |= S_IXUSR;
    } else if (i == 'S') {
        mode |= S_ISUID;
    } else if (i == 's') {
        mode |= S_IXUSR | S_ISUID;
    } else if (i != '-') {
        fprintf(stderr, "*** Invalid mode string: %s\n", input);
        return mode;
    }

    /* group */
    i = input[4];
    if (i == 'r') {
        mode |= S_IRGRP;
    } else if (i != '-') {
        fprintf(stderr, "*** Invalid mode string: %s\n", input);
        return mode;
    }

    i = input[5];
    if (i == 'w') {
        mode |= S_IWGRP;
    } else if (i != '-') {
        fprintf(stderr, "*** Invalid mode string: %s\n", input);
        return mode;
    }

    i = input[6];
    if (i == 'x') {
        mode |= S_IXGRP;
    } else if (i == 'S') {
        mode |= S_ISGID;
    } else if (i == 's') {
        mode |= S_IXGRP | S_ISGID;
    } else if (i != '-') {
        fprintf(stderr, "*** Invalid mode string: %s\n", input);
        return mode;
    }

    /* other */
    i = input[7];
    if (i == 'r') {
        mode |= S_IROTH;
    } else if (i != '-') {
        fprintf(stderr, "*** Invalid mode string: %s\n", input);
        return mode;
    }

    i = input[8];
    if (i == 'w') {
        mode |= S_IWOTH;
    } else if (i != '-') {
        fprintf(stderr, "*** Invalid mode string: %s\n", input);
        return mode;
    }

    i = input[9];
    if (i == 'x') {
        mode |= S_IXOTH;
    } else if (i == 'T') {
        mode |= S_ISVTX;
    } else if (i == 't') {
        mode |= S_IXOTH | S_ISVTX;
    } else if (i != '-') {
        fprintf(stderr, "*** Invalid mode string: %s\n", input);
        return mode;
    }

    return mode;
}

/*
 * Read either the main configuration file or a configuration file
 * overlay (profile) and populate the struct rpminspect members.
 */
static int read_cfgfile(dictionary *cfg, struct rpminspect *ri, const char *filename, const bool overlay)
{
    const char *tmp = NULL;

    assert(cfg != NULL);
    assert(ri != NULL);
    assert(filename != NULL);

    /* These settings can only appear in the main config file */
    if (!overlay) {
        tmp = iniparser_getstring(cfg, "common:workdir", NULL);
        if (tmp) {
            free(ri->workdir);
            ri->workdir = strdup(tmp);
        }

        tmp = iniparser_getstring(cfg, "common:profiledir", NULL);
        if (tmp) {
            free(ri->profiledir);
            ri->profiledir = strdup(tmp);
        }
    }

    tmp = iniparser_getstring(cfg, "koji:hub", NULL);
    if (tmp) {
        free(ri->kojihub);
        ri->kojihub = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "koji:download_ursine", NULL);
    if (tmp) {
        free(ri->kojiursine);
        ri->kojiursine = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "koji:download_mbs", NULL);
    if (tmp) {
        free(ri->kojimbs);
        ri->kojimbs = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "vendor-data:licensedb", NULL);
    if (tmp) {
        free(ri->licensedb);
        ri->licensedb = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "vendor-data:stat_whitelist_dir", NULL);
    if (tmp) {
        free(ri->stat_whitelist_dir);
        ri->stat_whitelist_dir = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "tests:badwords", NULL);
    if (tmp) {
        parse_list(tmp, &ri->badwords);
    }

    tmp = iniparser_getstring(cfg, "tests:vendor", NULL);
    if (tmp) {
        free(ri->vendor);
        ri->vendor = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "tests:buildhost_subdomain", NULL);
    if (tmp) {
        parse_list(tmp, &ri->buildhost_subdomain);
    }

    tmp = iniparser_getstring(cfg, "tests:security_path_prefix", NULL);
    if (tmp) {
        parse_list(tmp, &ri->security_path_prefix);
    }

    tmp = iniparser_getstring(cfg, "tests:header_file_extensions", NULL);
    if (tmp) {
        parse_list(tmp, &ri->header_file_extensions);
    }

    tmp = iniparser_getstring(cfg, "tests:forbidden_path_prefixes", NULL);
    if (tmp) {
        parse_list(tmp, &ri->forbidden_path_prefixes);
    }

    tmp = iniparser_getstring(cfg, "tests:forbidden_path_suffixes", NULL);
    if (tmp) {
        parse_list(tmp, &ri->forbidden_path_suffixes);
    }

    tmp = iniparser_getstring(cfg, "tests:forbidden_directories", NULL);
    if (tmp) {
        parse_list(tmp, &ri->forbidden_directories);
    }

    tmp = iniparser_getstring(cfg, "tests:elf_ipv6_blacklist", NULL);
    if (tmp) {
        parse_list(tmp, &ri->ipv6_blacklist);
    }

    /* If any of the regular expressions fail to compile, stop and return failure */
    if (add_regex(cfg, "tests:elf_path_include", &ri->elf_path_include) != 0) {
        return -1;
    }

    if (add_regex(cfg, "tests:elf_path_exclude", &ri->elf_path_exclude) != 0) {
        return -1;
    }

    if (add_regex(cfg, "tests:manpage_path_include", &ri->manpage_path_include) != 0) {
        return -1;
    }

    if (add_regex(cfg, "tests:manpage_path_exclude", &ri->manpage_path_exclude) != 0) {
        return -1;
    }

    if (add_regex(cfg, "tests:xml_path_include", &ri->xml_path_include) != 0) {
        return -1;
    }

    if (add_regex(cfg, "tests:xml_path_exclude", &ri->xml_path_exclude) != 0) {
        return -1;
    }

    tmp = iniparser_getstring(cfg, "tests:desktop_entry_files_dir", NULL);
    if (tmp) {
        free(ri->desktop_entry_files_dir);
        ri->desktop_entry_files_dir = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "tests:desktop_icon_paths", NULL);
    if (tmp) {
        parse_list(tmp, &ri->desktop_icon_paths);
    }

    tmp = iniparser_getstring(cfg, "tests:bin_paths", NULL);
    if (tmp) {
        parse_list(tmp, &ri->bin_paths);
    }

    tmp = iniparser_getstring(cfg, "tests:bin_owner", NULL);
    if (tmp) {
        free(ri->bin_owner);
        ri->bin_owner = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "tests:bin_group", NULL);
    if (tmp) {
        free(ri->bin_group);
        ri->bin_group = strdup(tmp);
    }

    tmp = iniparser_getstring(cfg, "tests:forbidden_owners", NULL);
    if (tmp) {
        parse_list(tmp, &ri->forbidden_owners);
    }

    tmp = iniparser_getstring(cfg, "tests:forbidden_groups", NULL);
    if (tmp) {
        parse_list(tmp, &ri->forbidden_groups);
    }

    tmp = iniparser_getstring(cfg, "tests:shells", NULL);
    if (tmp) {
        parse_list(tmp, &ri->shells);
    }

    tmp = iniparser_getstring(cfg, "specname:match", NULL);
    if (tmp) {
        if (!strcasecmp(tmp, "full")) {
            ri->specmatch = MATCH_FULL;
        } else if (!strcasecmp(tmp, "prefix")) {
            ri->specmatch = MATCH_PREFIX;
        } else if (!strcasecmp(tmp, "suffix")) {
            ri->specmatch = MATCH_SUFFIX;
        } else {
            fprintf(stderr, "*** Invalid specname:match setting in %s: %s\n", filename, tmp);
            fprintf(stderr, "*** Defaulting to 'full' matching.\n");
            ri->specmatch = MATCH_FULL;
        }
    }

    tmp = iniparser_getstring(cfg, "specname:primary", NULL);
    if (tmp) {
        if (!strcasecmp(tmp, "name")) {
            ri->specprimary = PRIMARY_NAME;
        } else if (!strcasecmp(tmp, "basename")) {
            ri->specprimary = PRIMARY_BASENAME;
        } else {
            fprintf(stderr, "*** Invalid specname:primary setting in %s: %s\n", filename, tmp);
            fprintf(stderr, "*** Defaulting to 'name' primary setting.\n");
            ri->specprimary = PRIMARY_NAME;
        }
    }

/* XXX */
    /* if a jvm major versions exist, collect those in to a hash table */
    read_mapping(cfg, "javabytecode", &ri->jvm_table, &ri->jvm_keys);

    /* if an annocheck section exists, collect those in to a hash table */
    read_mapping(cfg, "annocheck", &ri->annocheck_table, &ri->annocheck_keys);

    /* if a products section exists, collect those in to a hash table */
    read_mapping(cfg, "products", &ri->products, &ri->product_keys);
/* XXX */

    return 0;
}

/*
 * Initialize the stat-whitelist for the given product release.  If
 * the file cannot be found, return false.
 */
bool init_stat_whitelist(struct rpminspect *ri) {
    char *filename = NULL;
    FILE *input = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread = 0;
    char *token = NULL;
    char *fnpart = NULL;
    stat_whitelist_field_t field = MODE;
    stat_whitelist_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(ri->stat_whitelist_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->stat_whitelist) {
        return true;
    }

    /* the actual stat-whitelist file */
    xasprintf(&filename, "%s/%s", ri->stat_whitelist_dir, ri->product_release);
    assert(filename != NULL);

    input = fopen(filename, "r");
    free(filename);

    if (input == NULL) {
        return false;
    }

    /* initialize the list */
    ri->stat_whitelist = calloc(1, sizeof(*(ri->stat_whitelist)));
    assert(ri->stat_whitelist != NULL);
    TAILQ_INIT(ri->stat_whitelist);

    /* add all the entries to the stat-whitelist */
    while ((nread = getline(&line, &len, input)) != -1) {
        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r') {
            free(line);
            line = NULL;
            continue;
        }

        /* trim line ending characters */
        line[strcspn(line, "\r\n")] = '\0';

        /* initialize a new list entry */
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);

        /* read the fields */
        while ((token = strsep(&line, " \t")) != NULL) {
            /* might be lots of space between fields */
            if (*token == '\0') {
                continue;
            }

            /* copy the field in to the correct struct member */
            if (field == MODE) {
                entry->mode = parse_mode(token);
            } else if (field == OWNER) {
                entry->owner = strdup(token);
            } else if (field == GROUP) {
                entry->group = strdup(token);
            } else if (field == FILENAME) {
                fnpart = token;

                /* trim leading slashes since we compare to localpath later */
                while (*token != '/' && *token != '\0') {
                    token++;
                }

                if (*token == '\0') {
                    /* this is an invalid entry in the stat-whitelist */
                    fprintf(stderr, "*** Invalid filename in the stat-whitelist: %s\n", fnpart);
                    fprintf(stderr, "*** From this invalid line:\n");
                    fprintf(stderr, "***     %s\n", line);

                    free(entry->owner);
                    free(entry->group);
                    free(entry);
                    entry = NULL;
                } else {
                    entry->filename = strdup(token);
                }

                break;     /* nothing should come after this field */
            }

            field++;
        }

        /* add the entry */
        if (entry != NULL) {
            TAILQ_INSERT_TAIL(ri->stat_whitelist, entry, items);
        }

        /* clean up */
        free(line);
        line = NULL;
    }

    return true;
}

/*
 * Initialize a struct rpminspect.  Called by applications using
 * librpminspect before they began calling library functions.
 * Return 0 on success, -1 on failure.
 */
int init_rpminspect(struct rpminspect *ri, const char *cfgfile, const char *profile)
{
    int ret = 0;
    dictionary *cfg = NULL;
    uint64_t tests = 0;
    char *tmp = NULL;
    char *filename = NULL;

    assert(ri != NULL);
    memset(ri, 0, sizeof(*ri));

    /* Initialize the struct before reading files */
    ri->workdir = strdup(DEFAULT_WORKDIR);
    ri->profiledir = strdup(CFG_PROFILE_DIR);
    ri->kojihub = NULL;
    ri->kojiursine = NULL;
    ri->kojimbs = NULL;
    ri->licensedb = strdup(LICENSE_DB_FILE);
    ri->stat_whitelist_dir = strdup(STAT_WHITELIST_DIR);
    ri->stat_whitelist = NULL;
    ri->badwords = NULL;
    ri->vendor = NULL;
    ri->buildhost_subdomain = NULL;
    ri->security_path_prefix = NULL;
    ri->header_file_extensions = NULL;
    ri->forbidden_path_prefixes = NULL;
    ri->forbidden_path_suffixes = NULL;
    ri->forbidden_directories = NULL;
    ri->ipv6_blacklist = NULL;
    ri->elf_path_include = NULL;
    ri->elf_path_exclude = NULL;
    ri->manpage_path_include = NULL;
    ri->manpage_path_exclude = NULL;
    ri->xml_path_include = NULL;
    ri->xml_path_exclude = NULL;
    ri->desktop_entry_files_dir = strdup(DESKTOP_ENTRY_FILES_DIR);
    parse_list(DESKTOP_ICON_PATHS, &ri->desktop_icon_paths);
    parse_list(BIN_PATHS, &ri->bin_paths);
    ri->bin_owner = strdup(BIN_OWNER);
    ri->bin_group = strdup(BIN_GROUP);
    ri->forbidden_owners = NULL;
    ri->forbidden_groups = NULL;
    parse_list(SHELLS, &ri->shells);
    ri->specmatch = MATCH_FULL;
    ri->specprimary = PRIMARY_NAME;

    /* Store full path to the config file */
    ri->cfgfile = realpath(cfgfile, NULL);

    /* In case we have a missing configuration file, defaults all the way */
    if ((ri->cfgfile == NULL) || (access(ri->cfgfile, F_OK|R_OK) == -1)) {
        free(ri->cfgfile);
        ri->cfgfile = NULL;
        return 0;
    }

    /* Load the configuration file and get a dictionary */
    cfg = iniparser_load(ri->cfgfile);

    /* Read the main configuration file to get things started */
    ret = read_cfgfile(cfg, ri, ri->cfgfile, false);

    if (ret) {
        iniparser_freedict(cfg);
        return ret;
    }

    /* If a profile is specified, read an overlay config file */
    if (profile) {
        xasprintf(&tmp, "%s/%s.conf", ri->profiledir, profile);
        filename = realpath(tmp, NULL);

        if ((filename == NULL) || (access(filename, F_OK|R_OK) == -1)) {
            fprintf(stderr, "*** Unable to read profile '%s' from %s\n", profile, filename);
        } else {
            cfg = iniparser_load(filename);
            ret = read_cfgfile(cfg, ri, filename, true);
            iniparser_freedict(cfg);
        }
    }

    /* the rest of the members are used at runtime */
    ri->before = NULL;
    ri->after = NULL;
    ri->buildtype = KOJI_BUILD_RPM;
    ri->peers = init_rpmpeer();
    ri->worksubdir = NULL;
    ri->tests = ~tests;
    ri->results = NULL;
    ri->threshold = RESULT_VERIFY;
    ri->worst_result = RESULT_OK;
    ri->product_release = NULL;
    ri->arches = NULL;

    return 0;
}
