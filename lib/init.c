/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
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

#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <search.h>
#include <errno.h>
#include <err.h>
#include <yaml.h>
#include "rpminspect.h"

/* List defaults (not in constants.h to avoid cpp shenanigans) */

/**
 * @def BIN_PATHS
 * NULL terminated array of strings of paths where executable files
 * may reside.
 */
const char *BIN_PATHS[] = {"/bin", "/sbin", "/usr/bin", "/usr/sbin", NULL};

/**
 * @def SHELLS
 * NULL terminated array of strings of shells to use for syntax
 * checking (only the basename is needed).  All shells listed must
 * support the '-n' option for syntax checking.  The shell should exit
 * 0 if the syntax checker passes, non-zero otherwise.  The 'rc' shell
 * is an exception and has special handling in the 'shellsyntax'
 * inspection.
 */
const char *SHELLS[] = {"sh", "ksh", "zsh", "csh", "tcsh", "rc", "bash", NULL};

/* States used while reading the YAML config files */
enum {
    SYMBOL_NULL,
    SYMBOL_KEY,
    SYMBOL_VALUE,
    SYMBOL_ENTRY
};

/*
 * Sections of the config file.  The YAML reading loop determines if
 * these have been specified in the right order and in allowed
 * sections.  These values are used for state transitions while
 * reading the file.
 */
enum {
    BLOCK_NULL,
    BLOCK_ABIDIFF,
    BLOCK_ADDEDFILES,
    BLOCK_ANNOCHECK,
    BLOCK_BADWORDS,
    BLOCK_BIN_PATHS,
    BLOCK_BUILDHOST_SUBDOMAIN,
    BLOCK_CHANGEDFILES,
    BLOCK_COMMON,
    BLOCK_DESKTOP,
    BLOCK_ELF,
    BLOCK_FILESIZE,
    BLOCK_FORBIDDEN_GROUPS,
    BLOCK_FORBIDDEN_OWNERS,
    BLOCK_FORBIDDEN_PATH_PREFIXES,
    BLOCK_FORBIDDEN_PATH_SUFFIXES,
    BLOCK_FORBIDDEN_DIRECTORIES,
    BLOCK_HEADER_FILE_EXTENSIONS,
    BLOCK_IGNORE,
    BLOCK_INSPECTIONS,
    BLOCK_FORBIDDEN_IPV6_FUNCTIONS,
    BLOCK_JAVABYTECODE,
    BLOCK_KERNEL_FILENAMES,
    BLOCK_KMIDIFF,
    BLOCK_KOJI,
    BLOCK_LTO,
    BLOCK_LTO_SYMBOL_NAME_PREFIXES,
    BLOCK_FILES,
    BLOCK_FORBIDDEN_PATHS,
    BLOCK_MANPAGE,
    BLOCK_METADATA,
    BLOCK_OWNERSHIP,
    BLOCK_PATHMIGRATION,
    BLOCK_PRODUCTS,
    BLOCK_SECURITY_PATH_PREFIX,
    BLOCK_SHELLS,
    BLOCK_SHELLSYNTAX,
    BLOCK_SPECNAME,
    BLOCK_VENDOR,
    BLOCK_XML
};

static int add_regex(const char *pattern, regex_t **regex_out)
{
    int reg_result;
    char *errbuf = NULL;
    size_t errbuf_size;

    if (pattern == NULL || *pattern == '\0') {
        return 0;
    }

    assert(regex_out);

    free_regex(*regex_out);
    *regex_out = calloc(1, sizeof(regex_t));
    assert(*regex_out != NULL);

    if ((reg_result = regcomp(*regex_out, pattern, REG_EXTENDED)) != 0) {
        /* Get the size needed for the error message */
        errbuf_size = regerror(reg_result, *regex_out, NULL, 0);
        errbuf = malloc(errbuf_size);
        assert(errbuf != NULL);

        regerror(reg_result, *regex_out, errbuf, errbuf_size);
        fprintf(stderr, _("*** Unable to compile regular expression %s: %s\n"), pattern, errbuf);

        /* Clean up and return error */
        free(errbuf);
        free(*regex_out);
        return -1;
    }

    return 0;
}

/**
 * Given a string_list_t and a string, add the string to the list.  If
 * the list is NULL, initialize it to start a new list.  Caller is
 * responsible for freeing all memory associated with the list.
 *
 * @param list Pointer to a string_list_t to add entry to.
 * @param s String to add to the string_list_t.
 */
void add_entry(string_list_t **list, const char *s)
{
    string_entry_t *entry = NULL;

    assert(list != NULL);
    assert(s != NULL);

    if (*list == NULL) {
        *list = calloc(1, sizeof(*(*list)));
        assert(*list != NULL);
        TAILQ_INIT(*list);
    } else {
        /* do not add entry if it exists in the list */
        TAILQ_FOREACH(entry, *list, items) {
            if (!strcmp(entry->data, s)) {
                return;
            }
        }
    }

    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);

    entry->data = strdup(s);
    assert(entry->data != NULL);

    TAILQ_INSERT_TAIL(*list, entry, items);

    return;
}

/**
 * Given a pair_list_t, initialize a new hash table (clearing the old
 * one if necessary) and migrate the data from the list to the hash
 * table.  Clear the incoming hash table list.  The caller is
 * responsible for freeing all memory allocated to these structures.
 * Also note this function is destructive and will clear and
 * reinitialize all parameters.
 *
 * NOTE: The incoming table entries will be removed and freed as the
 * data is migrated to the table and keys.  Before return, the
 * incoming table list will be freed.  The actual key and value
 * strings in each incoming table entry are not freed since they just
 * migrate over to the new structures, but the entries themselves are
 * removed and freed from the incoming table.
 *
 * @param incoming_table List of key=value pairs to turn in to a hash table.
 * @param table Destination hash table.
 * @param keys Destination list of hash table keys.
 */
void process_table(pair_list_t *incoming_table, struct hsearch_data **table, string_list_t **keys)
{
    int len = 0;
    ENTRY e;
    ENTRY *eptr;
    string_entry_t *entry = NULL;
    pair_entry_t *pair = NULL;

    assert(incoming_table != NULL);

    /* clear out any existing data */
    free_mapping(*table, *keys);

    /* length of the incoming list */
    TAILQ_FOREACH(pair, incoming_table, items) {
        len++;
    }

    /* set up new data structures */
    *table = calloc(1, sizeof(**table));
    assert(*table != NULL);

    if (hcreate_r(len * 1.25, *table) == 0) {
        free(*table);
        *table = NULL;
        fprintf(stderr, "*** hcreate_r() failure in process_table()\n");
        fflush(stderr);
    }

    *keys = calloc(1, sizeof(**keys));
    assert(*keys != NULL);
    TAILQ_INIT(*keys);

    /*
     * migrate the list to the table
     * NOTE: This will remove and free each incoming table entry, but does
     * not free the key and value strings since those just migrate to the
     * hash table.  The caller needs to free the entire incoming table on
     * return of this function.
     */
    while (!TAILQ_EMPTY(incoming_table)) {
        pair = TAILQ_FIRST(incoming_table);

        /* save the key */
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->data = pair->key;
        TAILQ_INSERT_TAIL(*keys, entry, items);

        /* add the hash table entry */
        e.key = entry->data;
        e.data = pair->value;
        hsearch_r(e, ENTER, &eptr, *table);

        /* delete this incoming entry */
        TAILQ_REMOVE(incoming_table, pair, items);
        free(pair);
    }

    free(incoming_table);
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
        fprintf(stderr, _("*** Invalid input string `%s`\n"), input);
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
        fprintf(stderr, _("*** Invalid mode string: %s\n"), input);
        return mode;
    }

    /* owner */
    i = input[1];
    if (i == 'r') {
        mode |= S_IRUSR;
    } else if (i != '-') {
        fprintf(stderr, _("*** Invalid mode string: %s\n"), input);
        return mode;
    }

    i = input[2];
    if (i == 'w') {
        mode |= S_IWUSR;
    } else if (i != '-') {
        fprintf(stderr, _("*** Invalid mode string: %s\n"), input);
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
        fprintf(stderr, _("*** Invalid mode string: %s\n"), input);
        return mode;
    }

    /* group */
    i = input[4];
    if (i == 'r') {
        mode |= S_IRGRP;
    } else if (i != '-') {
        fprintf(stderr, _("*** Invalid mode string: %s\n"), input);
        return mode;
    }

    i = input[5];
    if (i == 'w') {
        mode |= S_IWGRP;
    } else if (i != '-') {
        fprintf(stderr, _("*** Invalid mode string: %s\n"), input);
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
        fprintf(stderr, _("*** Invalid mode string: %s\n"), input);
        return mode;
    }

    /* other */
    i = input[7];
    if (i == 'r') {
        mode |= S_IROTH;
    } else if (i != '-') {
        fprintf(stderr, _("*** Invalid mode string: %s\n"), input);
        return mode;
    }

    i = input[8];
    if (i == 'w') {
        mode |= S_IWOTH;
    } else if (i != '-') {
        fprintf(stderr, _("*** Invalid mode string: %s\n"), input);
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
        fprintf(stderr, _("*** Invalid mode string: %s\n"), input);
        return mode;
    }

    return mode;
}

/*
 * Read either the main configuration file or a configuration file
 * overlay (profile) and populate the struct rpminspect members.
 */
static int read_cfgfile(struct rpminspect *ri, const char *filename)
{
    FILE *fp = NULL;
    yaml_parser_t parser;
    yaml_token_t token;
    bool read_stream = false;
    bool read_list = false;
    int symbol = SYMBOL_NULL;
    int block = BLOCK_NULL;
    int group = BLOCK_NULL;
    int oldblock = BLOCK_NULL;
    char *key = NULL;
    char *t = NULL;
    bool exclude = false;
    bool read_incoming_table = false;
    pair_list_t *incoming_table = NULL;
    pair_entry_t *incoming_pair = NULL;
    struct hsearch_data **dest_table = NULL;
    string_list_t **dest_keys = NULL;

    assert(ri != NULL);
    assert(filename != NULL);

    /* prepare a YAML parser */
    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, _("*** error initializing YAML parser for module metadata, unable to filter\n"));
        fflush(stderr);
        return -1;
    }

    /* open the config file */
    if ((fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, _("*** error opening %s: %s\n"), filename, strerror(errno));
        fflush(stderr);
        return -1;
    }

    /* tell the YAML parser to read the config file */
    yaml_parser_set_input_file(&parser, fp);

    do {
        yaml_parser_scan(&parser, &token);

        switch (token.type) {
            case YAML_STREAM_START_TOKEN:
                /* begin reading the config file */
                read_stream = true;
                break;
            case YAML_STREAM_END_TOKEN:
                /* stop reading the config file */
                read_stream = false;
                break;
            case YAML_KEY_TOKEN:
                symbol = SYMBOL_KEY;
                break;
            case YAML_VALUE_TOKEN:
                symbol = SYMBOL_VALUE;
                break;
            case YAML_BLOCK_SEQUENCE_START_TOKEN:
                read_list = true;
                break;
            case YAML_BLOCK_ENTRY_TOKEN:
                if (read_list) {
                    symbol = SYMBOL_ENTRY;
                }

                break;
            case YAML_BLOCK_END_TOKEN:
                if (read_list) {
                    read_list = false;
                }

                /*
                 * blocks under groups should be closed first but if
                 * there is no block set, we are closing the group
                 */
                if (group != BLOCK_NULL && block != BLOCK_NULL) {
                    block = BLOCK_NULL;
                } else if (group != BLOCK_NULL && block == BLOCK_NULL) {
                    group = BLOCK_NULL;
                }

                break;
            case YAML_BLOCK_MAPPING_START_TOKEN:
                break;
            case YAML_SCALAR_TOKEN:
                /* convert the value to a string for comparison and copying */
                t = bytes_to_str(token.data.scalar.value, token.data.scalar.length);

                /* determine which config file block we are in */
                if (key && read_stream) {
                    if (!strcmp(key, "common")) {
                        block = BLOCK_COMMON;
                    } else if (!strcmp(key, "koji")) {
                        block = BLOCK_KOJI;
                    } else if (!strcmp(key, "vendor")) {
                        block = BLOCK_VENDOR;
                    } else if (!strcmp(key, "inspections")) {
                        block = BLOCK_INSPECTIONS;
                    } else if (!strcmp(key, "products")) {
                        block = BLOCK_PRODUCTS;
                    } else if (!strcmp(key, "ignore")) {
                        block = BLOCK_IGNORE;
                    } else if (!strcmp(key, "security_path_prefix")) {
                        block = BLOCK_SECURITY_PATH_PREFIX;
                    } else if (!strcmp(key, "badwords")) {
                        block = BLOCK_BADWORDS;
                    } else if (!strcmp(key, "metadata")) {
                        group = BLOCK_METADATA;
                    } else if (group == BLOCK_METADATA && !strcmp(key, "buildhost_subdomain")) {
                        block = BLOCK_BUILDHOST_SUBDOMAIN;
                    } else if (!strcmp(key, "elf")) {
                        group = BLOCK_ELF;
                    } else if (group == BLOCK_ELF && !strcmp(key, "forbidden_ipv6_functions")) {
                        block = BLOCK_FORBIDDEN_IPV6_FUNCTIONS;
                    } else if (!strcmp(key, "manpage")) {
                        block = BLOCK_MANPAGE;
                    } else if (!strcmp(key, "xml")) {
                        block = BLOCK_XML;
                    } else if (!strcmp(key, "desktop")) {
                        block = BLOCK_DESKTOP;
                    } else if (!strcmp(key, "changedfiles")) {
                        group = BLOCK_CHANGEDFILES;
                    } else if (group == BLOCK_CHANGEDFILES && !strcmp(key, "header_file_extensions")) {
                        block = BLOCK_HEADER_FILE_EXTENSIONS;
                    } else if (!strcmp(key, "addedfiles")) {
                        group = BLOCK_ADDEDFILES;
                    } else if (group == BLOCK_ADDEDFILES) {
                        if (!strcmp(key, "forbidden_path_prefixes")) {
                            block = BLOCK_FORBIDDEN_PATH_PREFIXES;
                        } else if (!strcmp(key, "forbidden_path_suffixes")) {
                            block = BLOCK_FORBIDDEN_PATH_SUFFIXES;
                        } else if (!strcmp(key, "forbidden_directories")) {
                            block = BLOCK_FORBIDDEN_DIRECTORIES;
                        }
                    } else if (!strcmp(key, "ownership")) {
                        group = BLOCK_OWNERSHIP;
                    } else if (group == BLOCK_OWNERSHIP) {
                        if (!strcmp(key, "bin_paths")) {
                            block = BLOCK_BIN_PATHS;
                        } else if (!strcmp(key, "forbidden_owners")) {
                            block = BLOCK_FORBIDDEN_OWNERS;
                        } else if (!strcmp(key, "forbidden_groups")) {
                            block = BLOCK_FORBIDDEN_GROUPS;
                        }
                    } else if (!strcmp(key, "shellsyntax")) {
                        group = BLOCK_SHELLSYNTAX;
                    } else if (group == BLOCK_SHELLSYNTAX && !strcmp(key, "shells")) {
                        block = BLOCK_SHELLS;
                    } else if (!strcmp(key, "filesize")) {
                        block = BLOCK_FILESIZE;
                    } else if (!strcmp(key, "lto")) {
                        group = BLOCK_LTO;
                    } else if (group == BLOCK_LTO && !strcmp(key, "lto_symbol_name_prefixes")) {
                        block = BLOCK_LTO_SYMBOL_NAME_PREFIXES;
                    } else if (!strcmp(key, "specname")) {
                        block = BLOCK_SPECNAME;
                    } else if (!strcmp(key, "annocheck")) {
                        block = BLOCK_ANNOCHECK;
                    } else if (!strcmp(key, "javabytecode")) {
                        block = BLOCK_JAVABYTECODE;
                    } else if (!strcmp(key, "pathmigration")) {
                        block = BLOCK_PATHMIGRATION;
                    } else if (!strcmp(key, "files")) {
                        group = BLOCK_FILES;
                    } else if (group == BLOCK_FILES && !strcmp(key, "forbidden_paths")) {
                        block = BLOCK_FORBIDDEN_PATHS;
                    } else if (!strcmp(key, "abidiff")) {
                        block = BLOCK_ABIDIFF;
                    } else if (!strcmp(key, "kmidiff")) {
                        group = BLOCK_KMIDIFF;
                    } else if (group == BLOCK_KMIDIFF && !strcmp(key, "kernel_filenames")) {
                        block = BLOCK_KERNEL_FILENAMES;
                    }
                }

                if (symbol == SYMBOL_KEY) {
                    /* save keys because they determine where values go */
                    free(key);
                    key = strdup(t);
                } else if (symbol == SYMBOL_VALUE) {
                    /* process any block that was a hash table */
                    if (block != oldblock && read_incoming_table) {
                        process_table(incoming_table, dest_table, dest_keys);

                        /* reset the incoming table (process_table already removed entries) */
                        incoming_table = NULL;
                        read_incoming_table = false;

                        /* reset the destination table and keys for incoming hash tables */
                        dest_table = NULL;
                        dest_keys = NULL;
                    }

                    /* sort values in to the right settings based on the block */
                    if (block == BLOCK_COMMON) {
                        if (!strcmp(key, "workdir")) {
                            free(ri->workdir);
                            ri->workdir = strdup(t);
                        } else if (!strcmp(key, "profiledir")) {
                            free(ri->profiledir);
                            ri->profiledir = strdup(t);
                        }
                    } else if (block == BLOCK_KOJI) {
                        if (!strcmp(key, "hub")) {
                            free(ri->kojihub);
                            ri->kojihub = strdup(t);
                        } else if (!strcmp(key, "download_ursine")) {
                            free(ri->kojiursine);
                            ri->kojiursine = strdup(t);
                        } else if (!strcmp(key, "download_mbs")) {
                            free(ri->kojimbs);
                            ri->kojimbs = strdup(t);
                        }
                    } else if (group == BLOCK_METADATA) {
                        /*
                         * this block needs to come before BLOCK_VENDOR
                         * the key 'vendor' is used in two places
                         */
                        if (!strcmp(key, "vendor")) {
                            free(ri->vendor);
                            ri->vendor = strdup(t);
                        }
                    } else if (block == BLOCK_VENDOR) {
                        if (!strcmp(key, "vendor_data_dir")) {
                            free(ri->vendor_data_dir);
                            ri->vendor_data_dir = strdup(t);
                        } else if (!strcmp(key, "licensedb")) {
                            free(ri->licensedb);
                            ri->licensedb = strdup(t);
                        } else if (!strcmp(key, "favor_release")) {
                            if (!strcasecmp(t, "none")) {
                                ri->favor_release = FAVOR_NONE;
                            } else if (!strcasecmp(t, "oldest")) {
                                ri->favor_release = FAVOR_OLDEST;
                            } else if (!strcasecmp(t, "newest")) {
                                ri->favor_release = FAVOR_NEWEST;
                            }
                        }
                    } else if (block == BLOCK_SPECNAME) {
                        if (!strcmp(key, "match")) {
                            if (!strcasecmp(t, "full")) {
                                ri->specmatch = MATCH_FULL;
                            } else if (!strcasecmp(t, "prefix")) {
                                ri->specmatch = MATCH_PREFIX;
                            } else if (!strcasecmp(t, "suffix")) {
                                ri->specmatch = MATCH_SUFFIX;
                            } else {
                                ri->specmatch = MATCH_FULL;
                                fprintf(stderr, "*** unknown specname match setting '%s', defaulting to 'full'\n", t);
                                fflush(stderr);
                            }
                        } else if (!strcmp(key, "primary")) {
                            if (!strcasecmp(t, "name")) {
                                ri->specprimary = PRIMARY_NAME;
                            } else if (!strcasecmp(t, "filename")) {
                                ri->specprimary = PRIMARY_FILENAME;
                            } else {
                                ri->specprimary = PRIMARY_NAME;
                                fprintf(stderr, "*** unknown specname primary setting '%s', defaulting to 'name'\n", t);
                                fflush(stderr);
                            }
                        }
                    } else if (group == BLOCK_ELF) {
                        if (!strcmp(key, "include_path")) {
                            if (debug_mode) {
                                free(ri->elf_path_include_pattern);
                                ri->elf_path_include_pattern = strdup(t);
                            }

                            if (add_regex(t, &ri->elf_path_include) != 0) {
                                fprintf(stderr, "*** error reading elf include path\n");
                                fflush(stderr);
                            }
                        } else if (!strcmp(key, "exclude_path")) {
                            if (debug_mode) {
                                free(ri->elf_path_exclude_pattern);
                                ri->elf_path_exclude_pattern = strdup(t);
                            }

                            if (add_regex(t, &ri->elf_path_exclude) != 0) {
                                fprintf(stderr, "*** error reading elf exclude path\n");
                                fflush(stderr);
                            }
                        }
                    } else if (block == BLOCK_MANPAGE) {
                        if (!strcmp(key, "include_path")) {
                            if (debug_mode) {
                                free(ri->manpage_path_include_pattern);
                                ri->manpage_path_include_pattern = strdup(t);
                            }

                            if (add_regex(t, &ri->manpage_path_include) != 0) {
                                fprintf(stderr, "*** error reading man page include path\n");
                                fflush(stderr);
                            }
                        } else if (!strcmp(key, "exclude_path")) {
                            if (debug_mode) {
                                free(ri->manpage_path_exclude_pattern);
                                ri->manpage_path_exclude_pattern = strdup(t);
                            }

                            if (add_regex(t, &ri->manpage_path_exclude) != 0) {
                                fprintf(stderr, "*** error reading man page exclude path\n");
                                fflush(stderr);
                            }
                        }
                    } else if (block == BLOCK_XML) {
                        if (!strcmp(key, "include_path")) {
                            if (debug_mode) {
                                free(ri->xml_path_include_pattern);
                                ri->xml_path_include_pattern = strdup(t);
                            }

                            if (add_regex(t, &ri->xml_path_include) != 0) {
                                fprintf(stderr, "*** error reading xml include path\n");
                                fflush(stderr);
                            }
                        } else if (!strcmp(key, "exclude_path")) {
                            if (debug_mode) {
                                free(ri->xml_path_exclude_pattern);
                                ri->xml_path_exclude_pattern = strdup(t);
                            }

                            if (add_regex(t, &ri->xml_path_exclude) != 0) {
                                fprintf(stderr, "*** error reading xml exclude path\n");
                                fflush(stderr);
                            }
                        }
                    } else if (block == BLOCK_DESKTOP) {
                        if (!strcmp(key, "desktop_entry_files_dir")) {
                            free(ri->desktop_entry_files_dir);
                            ri->desktop_entry_files_dir = strdup(t);
                        }
                    } else if (group == BLOCK_OWNERSHIP) {
                        if (!strcmp(key, "bin_owner")) {
                            free(ri->bin_owner);
                            ri->bin_owner = strdup(t);
                        } else if (!strcmp(key, "bin_group")) {
                            free(ri->bin_group);
                            ri->bin_group = strdup(t);
                        }
                    } else if (block == BLOCK_FILESIZE) {
                        if (!strcmp(key, "size_threshold")) {
                            ri->size_threshold = strtol(t, 0, 10);

                            if ((ri->size_threshold == LONG_MIN || ri->size_threshold == LONG_MAX) && errno == ERANGE) {
                                warn("strtol()");
                                ri->size_threshold = 0;
                            }
                        }
                    } else if (block == BLOCK_ANNOCHECK || block == BLOCK_JAVABYTECODE || block == BLOCK_PATHMIGRATION || block == BLOCK_PRODUCTS) {
                        if (incoming_table == NULL) {
                            incoming_table = calloc(1, sizeof(*incoming_table));
                            assert(incoming_table != NULL);
                            TAILQ_INIT(incoming_table);
                        }

                        incoming_pair = calloc(1, sizeof(*incoming_pair));
                        incoming_pair->key = strdup(key);
                        incoming_pair->value = strdup(t);
                        TAILQ_INSERT_TAIL(incoming_table, incoming_pair, items);

                        read_incoming_table = true;
                        oldblock = block;

                        if (block == BLOCK_ANNOCHECK) {
                            dest_table = &ri->annocheck;
                            dest_keys = &ri->annocheck_keys;
                        } else if (block == BLOCK_JAVABYTECODE) {
                            dest_table = &ri->jvm;
                            dest_keys = &ri->jvm_keys;
                        } else if (block == BLOCK_PATHMIGRATION) {
                            dest_table = &ri->pathmigration;
                            dest_keys = &ri->pathmigration_keys;
                        } else if (block == BLOCK_PRODUCTS) {
                            dest_table = &ri->products;
                            dest_keys = &ri->product_keys;
                        }
                    } else if (block == BLOCK_INSPECTIONS) {
                        if (!strcasecmp(t, "on")) {
                            exclude = false;
                        } else if (!strcasecmp(t, "off")) {
                            exclude = true;
                        } else {
                            exclude = false;
                            fprintf(stderr, "*** inspection flag must be 'on' or 'off', ignoring for '%s'\n", key);
                            fflush(stderr);
                        }

                        if (!process_inspection_flag(key, exclude, &ri->tests)) {
                            fprintf(stderr, _("*** Unknown inspection: `%s`\n"), key);
                            fflush(stderr);
                            exit(RI_PROGRAM_ERROR);
                        }
                    } else if (block == BLOCK_ABIDIFF) {
                        if (!strcmp(key, "suppression_file")) {
                            free(ri->abidiff_suppression_file);
                            ri->abidiff_suppression_file = strdup(t);
                        } else if (!strcmp(key, "debuginfo_path")) {
                            free(ri->abidiff_debuginfo_path);
                            ri->abidiff_debuginfo_path = strdup(t);
                        } else if (!strcmp(key, "include_path")) {
                            free(ri->abidiff_include_path);
                            ri->abidiff_include_path = strdup(t);
                        } else if (!strcmp(key, "extra_args")) {
                            free(ri->abidiff_extra_args);
                            ri->abidiff_extra_args = strdup(t);
                        } else if (!strcmp(key, "security_level_threshold")) {
                            ri->abi_security_threshold = strtol(t, 0, 10);

                            if ((ri->abi_security_threshold == LONG_MIN || ri->abi_security_threshold == LONG_MAX) && errno == ERANGE) {
                                warn("strtol()");
                                ri->abi_security_threshold = DEFAULT_ABI_SECURITY_THRESHOLD;
                            }
                        }
                    } else if (group == BLOCK_KMIDIFF) {
                        if (!strcmp(key, "suppression_file")) {
                            free(ri->kmidiff_suppression_file);
                            ri->kmidiff_suppression_file = strdup(t);
                        } else if (!strcmp(key, "debuginfo_path")) {
                            free(ri->kmidiff_debuginfo_path);
                            ri->kmidiff_debuginfo_path = strdup(t);
                        } else if (!strcmp(key, "extra_args")) {
                            free(ri->kmidiff_extra_args);
                            ri->kmidiff_extra_args = strdup(t);
                        } else if (!strcmp(key, "kmi_ignore_pattern")) {
                            free(ri->kmi_ignore_pattern);
                            ri->kmi_ignore_pattern = strdup(t);
                        }
                    }
                } else if (symbol == SYMBOL_ENTRY) {
                    if (block == BLOCK_BADWORDS) {
                        add_entry(&ri->badwords, t);
                    } else if (block == BLOCK_SECURITY_PATH_PREFIX) {
                        add_entry(&ri->security_path_prefix, t);
                    } else if (block == BLOCK_BUILDHOST_SUBDOMAIN) {
                        add_entry(&ri->buildhost_subdomain, t);
                    } else if (block == BLOCK_HEADER_FILE_EXTENSIONS) {
                        add_entry(&ri->header_file_extensions, t);
                    } else if (block == BLOCK_FORBIDDEN_PATH_PREFIXES) {
                        add_entry(&ri->forbidden_path_prefixes, t);
                    } else if (block == BLOCK_FORBIDDEN_PATH_SUFFIXES) {
                        add_entry(&ri->forbidden_path_suffixes, t);
                    } else if (block == BLOCK_FORBIDDEN_DIRECTORIES) {
                        add_entry(&ri->forbidden_directories, t);
                    } else if (block == BLOCK_FORBIDDEN_IPV6_FUNCTIONS) {
                        add_entry(&ri->forbidden_ipv6_functions, t);
                    } else if (block == BLOCK_BIN_PATHS) {
                        add_entry(&ri->bin_paths, t);
                    } else if (block == BLOCK_FORBIDDEN_OWNERS) {
                        add_entry(&ri->forbidden_owners, t);
                    } else if (block == BLOCK_FORBIDDEN_GROUPS) {
                        add_entry(&ri->forbidden_groups, t);
                    } else if (block == BLOCK_IGNORE) {
                        add_entry(&ri->ignores, t);
                    } else if (block == BLOCK_SHELLS) {
                        add_entry(&ri->shells, t);
                    } else if (block == BLOCK_LTO_SYMBOL_NAME_PREFIXES) {
                        add_entry(&ri->lto_symbol_name_prefixes, t);
                    } else if (block == BLOCK_FORBIDDEN_PATHS) {
                        add_entry(&ri->forbidden_paths, t);
                    } else if (block == BLOCK_KERNEL_FILENAMES) {
                        add_entry(&ri->kernel_filenames, t);
                    }
                }

                free(t);
                break;
            default:
                break;
        }

        if (token.type != YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
        }
    } while (token.type != YAML_STREAM_END_TOKEN);

    /* destroy the YAML parser, close the input file */
    yaml_parser_delete(&parser);

    if (fclose(fp) != 0) {
        fprintf(stderr, _("*** error ening %s: %s\n"), filename, strerror(errno));
        fflush(stderr);
        return -1;
    }

    /* process any final block that was a hash table */
    if (read_incoming_table) {
        process_table(incoming_table, dest_table, dest_keys);
    }

    /* clean up */
    free(key);

    return 0;
}

/*
 * Initialize the fileinfo list for the given product release.  If the
 * file cannot be found, return false.
 */
bool init_fileinfo(struct rpminspect *ri) {
    char *filename = NULL;
    FILE *input = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread = 0;
    char *token = NULL;
    char *fnpart = NULL;
    fileinfo_field_t field = MODE;
    fileinfo_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->fileinfo) {
        return true;
    }

    /* the actual fileinfo file */
    xasprintf(&filename, "%s/%s/%s", ri->vendor_data_dir, FILEINFO_DIR, ri->product_release);
    assert(filename != NULL);

    input = fopen(filename, "r");
    free(filename);

    if (input == NULL) {
        return false;
    }

    /* initialize the list */
    ri->fileinfo = calloc(1, sizeof(*(ri->fileinfo)));
    assert(ri->fileinfo != NULL);
    TAILQ_INIT(ri->fileinfo);

    /* add all the entries to the fileinfo list */
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
                    /* this is an invalid entry in the fileinfo list */
                    fprintf(stderr, _("*** Invalid filename in the fileinfo list: %s\n"), fnpart);
                    fprintf(stderr, _("*** From this invalid line:\n"));
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
            TAILQ_INSERT_TAIL(ri->fileinfo, entry, items);
        }

        /* clean up */
        free(line);
        line = NULL;
        field = MODE;
    }

    if (fclose(input) == -1) {
        warn("fclose()");
    }

    return true;
}

/*
 * Initialize the caps list for the given product release.  If the
 * file cannot be found, return false.
 */
bool init_caps(struct rpminspect *ri)
{
    char *filename = NULL;
    FILE *input = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread = 0;
    char *token = NULL;
    caps_field_t field = PACKAGE;
    caps_filelist_t *files = NULL;
    caps_entry_t *entry = NULL;
    caps_filelist_entry_t *filelist_entry = NULL;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->caps) {
        return true;
    }

    /* the actual caps list file */
    xasprintf(&filename, "%s/%s/%s", ri->vendor_data_dir, CAPABILITIES_DIR, ri->product_release);
    assert(filename != NULL);
    input = fopen(filename, "r");
    free(filename);

    if (input == NULL) {
        return false;
    }

    /* initialize the list */
    ri->caps = calloc(1, sizeof(*(ri->caps)));
    assert(ri->caps != NULL);
    TAILQ_INIT(ri->caps);

    /* add all the entries to the caps list */
    while ((nread = getline(&line, &len, input)) != -1) {
        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r') {
            free(line);
            line = NULL;
            continue;
        }

        /* trim line ending characters */
        line[strcspn(line, "\r\n")] = '\0';

        /* read the fields */
        while ((token = strsep(&line, " \t")) != NULL) {
            /* might be lots of space between fields */
            if (*token == '\0') {
                continue;
            }

            /* take action on each field */
            if (field == PACKAGE) {
                /* this package may exist in the list */
                TAILQ_FOREACH(entry, ri->caps, items) {
                    if (!strcmp(entry->pkg, token)) {
                        files = entry->files;
                        break;
                    }
                }

                if (TAILQ_EMPTY(ri->caps) || files == NULL) {
                    /* new package for the list */
                    entry = calloc(1, sizeof(*entry));
                    assert(entry != NULL);
                    entry->pkg = strdup(token);
                    entry->files = calloc(1, sizeof(*entry->files));
                    assert(entry->files != NULL);
                    TAILQ_INIT(entry->files);
                    TAILQ_INSERT_TAIL(ri->caps, entry, items);
                    files = entry->files;
                }

                filelist_entry = calloc(1, sizeof(*filelist_entry));
                assert(filelist_entry != NULL);
            } else if (field == FILEPATH) {
                filelist_entry->path = strdup(token);
            } else if (field == CAPABILITIES) {
                filelist_entry->caps = strdup(token);
            }

            field++;
        }

        /* add the entry */
        if (filelist_entry != NULL) {
            TAILQ_INSERT_TAIL(files, filelist_entry, items);
        }

        /* clean up */
        free(line);
        line = NULL;
        files = NULL;
        field = PACKAGE;
    }

    return true;
}

/*
 * Initialize a struct rpminspect.  Called by applications using
 * librpminspect before they began calling library functions.  If ri
 * passed in is NULL, the function will allocate and initialize a new
 * struct rpminspect.  The caller is responsible for freeing the
 * struct rpminspect.  Return 0 on success, -1 on failure.
 */
struct rpminspect *init_rpminspect(struct rpminspect *ri, const char *cfgfile, const char *profile)
{
    int i = 0;
    char *tmp = NULL;
    char *filename = NULL;
    char *kernelnames[] = KERNEL_FILENAMES;
    string_entry_t *cfg = NULL;

    /* Only initialize if we were given NULL for ri */
    if (ri == NULL) {
        ri = calloc(1, sizeof(*ri));
        assert(ri != NULL);

        /* Initialize the struct before reading files */
        ri->workdir = strdup(DEFAULT_WORKDIR);
        ri->vendor_data_dir = strdup(VENDOR_DATA_DIR);
        ri->favor_release = FAVOR_NONE;
        ri->tests = ~0;
        ri->desktop_entry_files_dir = strdup(DESKTOP_ENTRY_FILES_DIR);
        ri->bin_paths = list_from_array(BIN_PATHS);
        ri->bin_owner = strdup(BIN_OWNER);
        ri->bin_group = strdup(BIN_GROUP);
        ri->shells = list_from_array(SHELLS);
        ri->specmatch = MATCH_FULL;
        ri->specprimary = PRIMARY_NAME;
        ri->abidiff_suppression_file = strdup(ABI_SUPPRESSION_FILE);
        ri->abidiff_debuginfo_path = strdup(DEBUG_PATH);
        ri->abidiff_include_path = strdup(INCLUDE_PATH);
        ri->abi_security_threshold = DEFAULT_ABI_SECURITY_THRESHOLD;
        ri->kmidiff_suppression_file = strdup(ABI_SUPPRESSION_FILE);
        ri->kmidiff_debuginfo_path = strdup(DEBUG_PATH);

        /* Store full paths to all config files read */
        ri->cfgfiles = calloc(1, sizeof(*ri->cfgfiles));
        assert(ri->cfgfiles != NULL);
        TAILQ_INIT(ri->cfgfiles);
    }

    /* Read in the config file if we have it */
    if (cfgfile) {
        cfg = calloc(1, sizeof(*cfg));
        assert(cfg != NULL);
        cfg->data = realpath(cfgfile, NULL);

        /* In case we have a missing configuration file */
        if ((cfg->data == NULL) || (access(cfg->data, F_OK|R_OK) == -1)) {
            free(cfg->data);
            free(cfg);
            return NULL;
        }

        /* Read the main configuration file to get things started */
        i = read_cfgfile(ri, cfg->data);

        if (i) {
            warn(_("*** error reading '%s'\n"), cfg->data);
            free(cfg->data);
            free(cfg);
            return NULL;
        }

        /* Store this config file as one we read in */
        TAILQ_INSERT_TAIL(ri->cfgfiles, cfg, items);
    }

    /* If a profile is specified, read an overlay config file */
    if (profile) {
        xasprintf(&tmp, "%s/%s.yaml", ri->profiledir, profile);
        filename = realpath(tmp, NULL);

        if ((filename == NULL) || (access(filename, F_OK|R_OK) == -1)) {
            warn(_("*** Unable to read profile '%s' from %s\n"), profile, filename);
        } else {
            i = read_cfgfile(ri, filename);

            if (i) {
                warn(_("*** error reading '%s'\n"), filename);
                return NULL;
            }
        }

        free(tmp);
    }

    /* Initialize some lists if we did not get any config file data */
    if (ri->kernel_filenames == NULL) {
        ri->kernel_filenames = calloc(1, sizeof(*ri->kernel_filenames));
        assert(ri->kernel_filenames != NULL);
        TAILQ_INIT(ri->kernel_filenames);

        for(i = 0; kernelnames[i] != NULL; i++) {
            cfg = calloc(1, sizeof(*cfg));
            cfg->data = strdup(kernelnames[i]);
            assert(cfg->data != NULL);
            TAILQ_INSERT_TAIL(ri->kernel_filenames, cfg, items);
        }
    }

    /* the rest of the members are used at runtime */
    ri->buildtype = KOJI_BUILD_RPM;
    ri->peers = init_rpmpeer();
    ri->threshold = RESULT_VERIFY;
    ri->worst_result = RESULT_OK;

    return ri;
}
