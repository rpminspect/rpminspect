/*
 * Copyright © 2019 Red Hat, Inc.
 * Author(s): David Cantrell <dcantrell@redhat.com>
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
#include <errno.h>
#include <err.h>
#include <yaml.h>
#include "rpminspect.h"
#include "queue.h"
#include "uthash.h"

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
    BLOCK_BADFUNCS,
    BLOCK_BADWORDS,
    BLOCK_BIN_PATHS,
    BLOCK_BUILDHOST_SUBDOMAIN,
    BLOCK_CHANGEDFILES,
    BLOCK_COMMANDS,
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
    BLOCK_MIGRATED_PATHS,
    BLOCK_OWNERSHIP,
    BLOCK_PATCHES,
    BLOCK_PATCH_FILENAMES,
    BLOCK_PATHMIGRATION,
    BLOCK_PATHMIGRATION_EXCLUDED_PATHS,
    BLOCK_PRODUCTS,
    BLOCK_RUNPATH,
    BLOCK_RUNPATH_ALLOWED_PATHS,
    BLOCK_RUNPATH_ALLOWED_ORIGIN_PATHS,
    BLOCK_RUNPATH_ORIGIN_PREFIX_TRIM,
    BLOCK_SECURITY_PATH_PREFIX,
    BLOCK_SHELLS,
    BLOCK_SHELLSYNTAX,
    BLOCK_SPECNAME,
    BLOCK_VENDOR,
    BLOCK_XML,
    BLOCK_EMPTYRPM,
    BLOCK_EXPECTED_EMPTY_RPMS,
    BLOCK_TYPES
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
        warn("%s", errbuf);

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
static void add_entry(string_list_t **list, const char *s)
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
        if (list_contains(*list, s)) {
            return;
        }
    }

    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);

    entry->data = strdup(s);
    assert(entry->data != NULL);

    TAILQ_INSERT_TAIL(*list, entry, items);

    return;
}

/*
 * Given an inspection identifier from the config file reader and a
 * list value, add it to the per-inspection list of ignores.
 */
static void add_ignore(string_list_map_t **table, int i, char *s)
{
    char *inspection = NULL;
    string_list_map_t *entry = NULL;
    string_entry_t *ignore = NULL;

    assert(s != NULL);
    assert(i != BLOCK_NULL);

    /*
     * determine the inspection first.  we do not allow all
     * inspections to have per-inspection ignore lists.  invalid ones
     * are reported out.
     */
    if (i == BLOCK_ELF) {
        inspection = NAME_ELF;
    } else if (i == BLOCK_MANPAGE) {
        inspection = NAME_MANPAGE;
    } else if (i == BLOCK_XML) {
        inspection = NAME_XML;
    } else if (i == BLOCK_DESKTOP) {
        inspection = NAME_DESKTOP;
    } else if (i == BLOCK_CHANGEDFILES) {
        inspection = NAME_CHANGEDFILES;
    } else if (i == BLOCK_ADDEDFILES) {
        inspection = NAME_ADDEDFILES;
    } else if (i == BLOCK_OWNERSHIP) {
        inspection = NAME_OWNERSHIP;
    } else if (i == BLOCK_SHELLSYNTAX) {
        inspection = NAME_SHELLSYNTAX;
    } else if (i == BLOCK_FILESIZE) {
        inspection = NAME_FILESIZE;
    } else if (i == BLOCK_LTO) {
        inspection = NAME_LTO;
    } else if (i == BLOCK_ANNOCHECK) {
        inspection = NAME_ANNOCHECK;
    } else if (i == BLOCK_JAVABYTECODE) {
        inspection = NAME_JAVABYTECODE;
    } else if (i == BLOCK_PATHMIGRATION) {
        inspection = NAME_PATHMIGRATION;
    } else if (i == BLOCK_FILES) {
        inspection = NAME_FILES;
    } else if (i == BLOCK_ABIDIFF) {
        inspection = NAME_ABIDIFF;
    } else if (i == BLOCK_KMIDIFF) {
        inspection = NAME_KMIDIFF;
    } else if (i == BLOCK_BADFUNCS) {
        inspection = NAME_BADFUNCS;
    } else if (i == BLOCK_RUNPATH) {
        inspection = NAME_RUNPATH;
    } else if (i == BLOCK_TYPES) {
        inspection = NAME_TYPES;
    } else {
        warnx(_("*** ignore found in %d, value `%s'"), i, s);
        return;
    }

    DEBUG_PRINT("    add_ignore -> inspection=%s, s=|%s|\n", inspection, s);

    /* create the entry for the value list */
    ignore = calloc(1, sizeof(*ignore));
    assert(ignore != NULL);
    ignore->data = strdup(s);
    assert(ignore->data != NULL);

    /* look for the list first, add if not found */
    HASH_FIND_STR(*table, inspection, entry);

    if (entry == NULL) {
        /* start a new entry for this inspection */
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->key = strdup(inspection);
        entry->value = calloc(1, sizeof(*entry->value));
        assert(entry->value != NULL);
        TAILQ_INIT(entry->value);
        TAILQ_INSERT_TAIL(entry->value, ignore, items);
        HASH_ADD_KEYPTR(hh, *table, entry->key, strlen(entry->key), entry);
    } else {
        /* on the off chance we have an empty list of values */
        if (entry->value == NULL) {
            entry->value = calloc(1, sizeof(*entry->value));
            assert(entry->value != NULL);
            TAILQ_INIT(entry->value);
        }

        /* add to existing ignore list */
        TAILQ_INSERT_TAIL(entry->value, ignore, items);
    }

    return;
}

/**
 * Given a key and value, initialize a new hash table (clearing the old
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
 */
static void process_table(char *key, char *value, string_map_t **table)
{
    string_map_t *entry = NULL;

    assert(key != NULL);
    assert(value != NULL);

    /* look for the key first */
    HASH_FIND_STR(*table, key, entry);

    if (entry == NULL) {
        /* add the new key/value pair to the table */
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->key = strdup(key);
        entry->value = strdup(value);
        HASH_ADD_KEYPTR(hh, *table, entry->key, strlen(entry->key), entry);
    } else {
        /* entry found, replace the value */
        free(entry->value);
        entry->value = strdup(value);
    }

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
        warn(_("invalid input string `%s`"), input);
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
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    /* owner */
    i = input[1];
    if (i == 'r') {
        mode |= S_IRUSR;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    i = input[2];
    if (i == 'w') {
        mode |= S_IWUSR;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
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
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    /* group */
    i = input[4];
    if (i == 'r') {
        mode |= S_IRGRP;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    i = input[5];
    if (i == 'w') {
        mode |= S_IWGRP;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
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
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    /* other */
    i = input[7];
    if (i == 'r') {
        mode |= S_IROTH;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    i = input[8];
    if (i == 'w') {
        mode |= S_IWOTH;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
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
        warnx(_("*** invalid mode string: %s"), input);
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
    unsigned int level = 0;
    char *key = NULL;
    char *t = NULL;
    bool exclude = false;

    assert(ri != NULL);
    assert(filename != NULL);

    /* prepare a YAML parser */
    if (!yaml_parser_initialize(&parser)) {
        warn("yaml_parser_initialize()");
        return -1;
    }

    /* open the config file */
    if ((fp = fopen(filename, "r")) == NULL) {
        warn("fopen()");
        return -1;
    }

    /* tell the YAML parser to read the config file */
    yaml_parser_set_input_file(&parser, fp);

    do {
        if (yaml_parser_scan(&parser, &token) == 0) {
            warnx(_("ignoring malformed %s configuration file: %s"), COMMAND_NAME, filename);
            return -1;
        }

        switch (token.type) {
            case YAML_STREAM_START_TOKEN:
                /* begin reading the config file */
                DEBUG_PRINT("YAML_STREAM_START_TOKEN\n");
                read_stream = true;
                break;
            case YAML_STREAM_END_TOKEN:
                /* stop reading the config file */
                DEBUG_PRINT("YAML_STREAM_END_TOKEN\n");
                read_stream = false;
                break;
            case YAML_KEY_TOKEN:
                DEBUG_PRINT("YAML_KEY_TOKEN -> SYMBOL_KEY\n");
                symbol = SYMBOL_KEY;
                break;
            case YAML_VALUE_TOKEN:
                DEBUG_PRINT("YAML_VALUE_TOKEN -> SYMBOL_VALUE\n");
                symbol = SYMBOL_VALUE;
                break;
            case YAML_BLOCK_SEQUENCE_START_TOKEN:
                DEBUG_PRINT("YAML_BLOCK_SEQUENCE_START_TOKEN\n");
                read_list = true;
                break;
            case YAML_BLOCK_ENTRY_TOKEN:
                DEBUG_PRINT("YAML_BLOCK_ENTRY_TOKEN\n");

                if (read_list) {
                    DEBUG_PRINT("    -> SYMBOL_ENTRY, block=%d, group=%d\n", block, group);
                    symbol = SYMBOL_ENTRY;
                }

                break;
            case YAML_BLOCK_END_TOKEN:
                DEBUG_PRINT("YAML_BLOCK_END_TOKEN\n");

                if (read_list && block == BLOCK_NULL && group != BLOCK_NULL) {
                    DEBUG_PRINT("    -> read_list=false\n");
                    read_list = false;
                }

                /*
                 * Go back up a level until we are at level 1.  The
                 * variable is initialized to 0 though we don't really
                 * use that level.  libyaml throws out two
                 * YAML_BLOCK_END_TOKENs when reading a stream.  The
                 * first one can be ignored so level 1 becomes our
                 * effective top level.
                 */
                if (level > 1) {
                    DEBUG_PRINT("    -> level--\n");
                    level--;
                }

                /* At the top?  Reset block indicator. */
                if (level == 1) {
                    DEBUG_PRINT("    -> block=BLOCK_NULL\n");
                    block = BLOCK_NULL;
                }

                break;
            case YAML_BLOCK_MAPPING_START_TOKEN:
                /* YAML is all endless blocks */
                DEBUG_PRINT("YAML_BLOCK_MAPPING_START_TOKEN\n");
                level++;
                break;
            case YAML_SCALAR_TOKEN:
                /* convert the value to a string for comparison and copying */
                t = bytes_to_str(token.data.scalar.value, token.data.scalar.length);
                DEBUG_PRINT("YAML_SCALAR_TOKEN -> key=%s\n", key);

                /* determine which config file block we are in */
                if (key && read_stream && block != BLOCK_INSPECTIONS) {
                    DEBUG_PRINT("    (top level)\n");

                    if (!strcmp(key, "common")) {
                        block = BLOCK_COMMON;
                        group = BLOCK_NULL;
                    } else if (!strcmp(key, "koji")) {
                        block = BLOCK_KOJI;
                        group = BLOCK_NULL;
                    } else if (!strcmp(key, "vendor") && group != BLOCK_METADATA) {
                        block = BLOCK_VENDOR;
                        group = BLOCK_NULL;
                    } else if (!strcmp(key, "commands")) {
                        block = BLOCK_COMMANDS;
                        group = BLOCK_NULL;
                    } else if (!strcmp(key, "inspections")) {
                        block = BLOCK_INSPECTIONS;
                        group = BLOCK_NULL;
                    } else if (!strcmp(key, "products")) {
                        block = BLOCK_NULL;
                        group = BLOCK_PRODUCTS;
                    /*
                     * All of these groups are listed here explicitly
                     * because we have a top-level 'ignore' block and
                     * then all of these blocks all an 'ignore'
                     * subblock.  We have to handle things this way so
                     * the config file parses correctly.
                     */
                    } else if (!strcmp(key, "ignore") && (group != BLOCK_ELF &&
                                                          group != BLOCK_MANPAGE &&
                                                          group != BLOCK_XML &&
                                                          group != BLOCK_DESKTOP &&
                                                          group != BLOCK_CHANGEDFILES &&
                                                          group != BLOCK_ADDEDFILES &&
                                                          group != BLOCK_OWNERSHIP &&
                                                          group != BLOCK_SHELLSYNTAX &&
                                                          group != BLOCK_FILESIZE &&
                                                          group != BLOCK_LTO &&
                                                          group != BLOCK_ANNOCHECK &&
                                                          group != BLOCK_JAVABYTECODE &&
                                                          group != BLOCK_PATHMIGRATION &&
                                                          group != BLOCK_FILES &&
                                                          group != BLOCK_ABIDIFF &&
                                                          group != BLOCK_KMIDIFF &&
                                                          group != BLOCK_BADFUNCS &&
                                                          group != BLOCK_RUNPATH &&
                                                          group != BLOCK_TYPES)) {
                        block = BLOCK_IGNORE;
                        group = BLOCK_NULL;
                    } else if (!strcmp(key, "security_path_prefix")) {
                        block = BLOCK_SECURITY_PATH_PREFIX;

                        if (group != BLOCK_SECURITY_PATH_PREFIX) {
                            list_free(ri->security_path_prefix, free);
                            ri->security_path_prefix = NULL;
                            group = BLOCK_SECURITY_PATH_PREFIX;
                        }
                    } else if (!strcmp(key, "badwords")) {
                        block = BLOCK_BADWORDS;

                        if (group != BLOCK_BADWORDS) {
                            list_free(ri->badwords, free);
                            ri->badwords = NULL;
                            group = BLOCK_BADWORDS;
                        }
                    } else if (!strcmp(key, NAME_METADATA)) {
                        block = BLOCK_NULL;
                        group = BLOCK_METADATA;
                    } else if (!strcmp(key, NAME_ELF)) {
                        block = BLOCK_NULL;
                        group = BLOCK_ELF;
                    } else if (!strcmp(key, NAME_MANPAGE)) {
                        block = BLOCK_NULL;
                        group = BLOCK_MANPAGE;
                    } else if (!strcmp(key, NAME_XML)) {
                        block = BLOCK_NULL;
                        group = BLOCK_XML;
                    } else if (!strcmp(key, NAME_DESKTOP)) {
                        block = BLOCK_NULL;
                        group = BLOCK_DESKTOP;
                    } else if (!strcmp(key, NAME_CHANGEDFILES)) {
                        block = BLOCK_NULL;
                        group = BLOCK_CHANGEDFILES;
                    } else if (!strcmp(key, NAME_ADDEDFILES)) {
                        block = BLOCK_NULL;
                        group = BLOCK_ADDEDFILES;
                    } else if (!strcmp(key, NAME_OWNERSHIP)) {
                        block = BLOCK_NULL;
                        group = BLOCK_OWNERSHIP;
                    } else if (!strcmp(key, NAME_SHELLSYNTAX)) {
                        block = BLOCK_NULL;
                        group = BLOCK_SHELLSYNTAX;
                    } else if (!strcmp(key, NAME_FILESIZE)) {
                        block = BLOCK_NULL;
                        group = BLOCK_FILESIZE;
                    } else if (!strcmp(key, NAME_LTO)) {
                        block = BLOCK_NULL;
                        group = BLOCK_LTO;
                    } else if (!strcmp(key, NAME_SPECNAME)) {
                        block = BLOCK_SPECNAME;
                        group = BLOCK_NULL;
                    } else if (!strcmp(key, NAME_ANNOCHECK)) {
                        block = BLOCK_NULL;
                        group = BLOCK_ANNOCHECK;
                    } else if (!strcmp(key, NAME_JAVABYTECODE)) {
                        block = BLOCK_NULL;
                        group = BLOCK_JAVABYTECODE;
                    } else if (!strcmp(key, NAME_PATHMIGRATION)) {
                        block = BLOCK_NULL;
                        group = BLOCK_PATHMIGRATION;
                    } else if (!strcmp(key, NAME_FILES)) {
                        block = BLOCK_NULL;
                        group = BLOCK_FILES;
                    } else if (!strcmp(key, NAME_ABIDIFF)) {
                        block = BLOCK_NULL;
                        group = BLOCK_ABIDIFF;
                    } else if (!strcmp(key, NAME_KMIDIFF)) {
                        block = BLOCK_NULL;
                        group = BLOCK_KMIDIFF;
                    } else if (!strcmp(key, NAME_PATCHES)) {
                        block = BLOCK_NULL;
                        group = BLOCK_PATCHES;
                    } else if (!strcmp(key, NAME_BADFUNCS)) {
                        block = BLOCK_BADFUNCS;

                        if (group != BLOCK_BADFUNCS) {
                            list_free(ri->bad_functions, free);
                            ri->bad_functions = NULL;
                            group = BLOCK_BADFUNCS;
                        }
                    } else if (!strcmp(key, NAME_RUNPATH)) {
                        block = BLOCK_NULL;
                        group = BLOCK_RUNPATH;
                    } else if (!strcmp(key, NAME_EMPTYRPM)) {
                        block = BLOCK_NULL;
                        group = BLOCK_EMPTYRPM;
                    } else if (!strcmp(key, NAME_TYPES)) {
                        block = BLOCK_NULL;
                        group = BLOCK_TYPES;
                    }
                }

                if (symbol == SYMBOL_KEY) {
                    /* save keys because they determine where values go */
                    free(key);
                    key = strdup(t);
                    DEBUG_PRINT("    -> SYMBOL_KEY=%s\n", key);

                    /* handle group subsection blocks here rather than above */
                    if (group == BLOCK_PATHMIGRATION || group == BLOCK_MIGRATED_PATHS || group == BLOCK_PATHMIGRATION_EXCLUDED_PATHS) {
                        if (!strcmp(key, "migrated_paths")) {
                            group = BLOCK_MIGRATED_PATHS;
                        } else if (!strcmp(key, "excluded_paths")) {
                            group = BLOCK_PATHMIGRATION_EXCLUDED_PATHS;
                            list_free(ri->pathmigration_excluded_paths, free);
                            ri->pathmigration_excluded_paths = NULL;
                        } else if (!strcmp(key, "ignore")) {
                            group = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_METADATA) {
                        if (!strcmp(key, "buildhost_subdomain")) {
                            block = BLOCK_BUILDHOST_SUBDOMAIN;
                            list_free(ri->buildhost_subdomain, free);
                            ri->buildhost_subdomain = NULL;
                        }
                    } else if (group == BLOCK_CHANGEDFILES) {
                        if (!strcmp(key, "header_file_extensions")) {
                            block = BLOCK_HEADER_FILE_EXTENSIONS;
                            list_free(ri->header_file_extensions, free);
                            ri->header_file_extensions = NULL;
                        } else if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_ADDEDFILES) {
                        if (!strcmp(key, "forbidden_path_prefixes")) {
                            block = BLOCK_FORBIDDEN_PATH_PREFIXES;
                            list_free(ri->forbidden_path_prefixes, free);
                            ri->forbidden_path_prefixes = NULL;
                        } else if (!strcmp(key, "forbidden_path_suffixes")) {
                            block = BLOCK_FORBIDDEN_PATH_SUFFIXES;
                            list_free(ri->forbidden_path_suffixes, free);
                            ri->forbidden_path_suffixes = NULL;
                        } else if (!strcmp(key, "forbidden_directories")) {
                            block = BLOCK_FORBIDDEN_DIRECTORIES;
                            list_free(ri->forbidden_directories, free);
                            ri->forbidden_directories = NULL;
                        } else if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_OWNERSHIP) {
                        if (!strcmp(key, "bin_paths")) {
                            block = BLOCK_BIN_PATHS;
                            list_free(ri->bin_paths, free);
                            ri->bin_paths = NULL;
                        } else if (!strcmp(key, "forbidden_owners")) {
                            block = BLOCK_FORBIDDEN_OWNERS;
                            list_free(ri->forbidden_owners, free);
                            ri->forbidden_owners = NULL;
                        } else if (!strcmp(key, "forbidden_groups")) {
                            block = BLOCK_FORBIDDEN_GROUPS;
                            list_free(ri->forbidden_groups, free);
                            ri->forbidden_groups = NULL;
                        } else if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_SHELLSYNTAX) {
                        if (!strcmp(key, "shells")) {
                            block = BLOCK_SHELLS;
                            list_free(ri->shells, free);
                            ri->shells = NULL;
                        } else if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_LTO) {
                        if (!strcmp(key, "lto_symbol_name_prefixes")) {
                            block = BLOCK_LTO_SYMBOL_NAME_PREFIXES;
                            list_free(ri->lto_symbol_name_prefixes, free);
                            ri->lto_symbol_name_prefixes = NULL;
                        } else if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_FILES) {
                        if (!strcmp(key, "forbidden_paths")) {
                            block = BLOCK_FORBIDDEN_PATHS;
                            list_free(ri->forbidden_paths, free);
                            ri->forbidden_paths = NULL;
                        } else if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_KMIDIFF) {
                        if (!strcmp(key, "kernel_filenames")) {
                            block = BLOCK_KERNEL_FILENAMES;
                            list_free(ri->kernel_filenames, free);
                            ri->kernel_filenames = NULL;
                        } else if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_PATCHES) {
                        if (!strcmp(key, "patch_ignore_list")) {
                            block = BLOCK_PATCH_FILENAMES;
                            list_free(ri->patch_ignore_list, free);
                            ri->patch_ignore_list = NULL;
                        }
                    } else if (group == BLOCK_RUNPATH) {
                        if (!strcmp(key, "allowed_paths")) {
                            block = BLOCK_RUNPATH_ALLOWED_PATHS;
                            list_free(ri->runpath_allowed_paths, free);
                            ri->runpath_allowed_paths = NULL;
                        } else if (!strcmp(key, "allowed_origin_paths")) {
                            block = BLOCK_RUNPATH_ALLOWED_ORIGIN_PATHS;
                            list_free(ri->runpath_allowed_origin_paths, free);
                            ri->runpath_allowed_origin_paths = NULL;
                        } else if (!strcmp(key, "origin_prefix_trim")) {
                            block = BLOCK_RUNPATH_ORIGIN_PREFIX_TRIM;
                            list_free(ri->runpath_origin_prefix_trim, free);
                            ri->runpath_origin_prefix_trim = NULL;
                        } else if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_ELF) {
                        if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_MANPAGE) {
                        if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_XML) {
                        if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_DESKTOP) {
                        if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_FILESIZE) {
                        if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_ANNOCHECK) {
                        if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_JAVABYTECODE) {
                        if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    } else if (block == BLOCK_BADFUNCS) {
                        if (!strcmp(key, "ignore")) {
                            group = BLOCK_BADFUNCS;
                            block = BLOCK_IGNORE;
                        }
                    } else if (group == BLOCK_EMPTYRPM) {
                        if (!strcmp(key, "expected_empty")) {
                            block = BLOCK_EXPECTED_EMPTY_RPMS;
                            list_free(ri->expected_empty_rpms, free);
                            ri->expected_empty_rpms = NULL;
                        }
                    } else if (group == BLOCK_TYPES) {
                        if (!strcmp(key, "ignore")) {
                            block = BLOCK_IGNORE;
                        }
                    }
                } else if (symbol == SYMBOL_VALUE) {
                    DEBUG_PRINT("    -> SYMBOL_VALUE=%s\n", key);

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
                    } else if (block == BLOCK_COMMON) {
                        if (!strcmp(key, "diff")) {
                            free(ri->commands.diff);
                            ri->commands.diff = strdup(t);
                        } else if (!strcmp(key, "diffstat")) {
                            free(ri->commands.diffstat);
                            ri->commands.diffstat = strdup(t);
                        } else if (!strcmp(key, "msgunfmt")) {
                            free(ri->commands.msgunfmt);
                            ri->commands.msgunfmt = strdup(t);
                        } else if (!strcmp(key, "desktop-file-validate")) {
                            free(ri->commands.desktop_file_validate);
                            ri->commands.desktop_file_validate = strdup(t);
                        } else if (!strcmp(key, "annocheck")) {
                            free(ri->commands.annocheck);
                            ri->commands.annocheck = strdup(t);
                        } else if (!strcmp(key, "abidiff")) {
                            free(ri->commands.abidiff);
                            ri->commands.abidiff = strdup(t);
                        } else if (!strcmp(key, "kmidiff")) {
                            free(ri->commands.kmidiff);
                            ri->commands.kmidiff = strdup(t);
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
                                warnx(_("*** unknown specname match setting '%s', defaulting to 'full'"), t);
                            }
                        } else if (!strcmp(key, "primary")) {
                            if (!strcasecmp(t, "name")) {
                                ri->specprimary = PRIMARY_NAME;
                            } else if (!strcasecmp(t, "filename")) {
                                ri->specprimary = PRIMARY_FILENAME;
                            } else {
                                ri->specprimary = PRIMARY_NAME;
                                warnx(_("*** unknown specname primary setting '%s', defaulting to 'name'"), t);
                            }
                        }
                    } else if (group == BLOCK_ELF) {
                        if (!strcmp(key, "include_path")) {
                            if (debug_mode) {
                                free(ri->elf_path_include_pattern);
                                ri->elf_path_include_pattern = strdup(t);
                            }

                            if (add_regex(t, &ri->elf_path_include) != 0) {
                                warn(_("error reading elf include path"));
                            }
                        } else if (!strcmp(key, "exclude_path")) {
                            if (debug_mode) {
                                free(ri->elf_path_exclude_pattern);
                                ri->elf_path_exclude_pattern = strdup(t);
                            }

                            if (add_regex(t, &ri->elf_path_exclude) != 0) {
                                warn(_("error reading elf exclude path"));
                            }
                        }
                    } else if (group == BLOCK_MANPAGE) {
                        if (!strcmp(key, "include_path")) {
                            if (debug_mode) {
                                free(ri->manpage_path_include_pattern);
                                ri->manpage_path_include_pattern = strdup(t);
                            }

                            if (add_regex(t, &ri->manpage_path_include) != 0) {
                                warn(_("error reading man page include path"));
                            }
                        } else if (!strcmp(key, "exclude_path")) {
                            if (debug_mode) {
                                free(ri->manpage_path_exclude_pattern);
                                ri->manpage_path_exclude_pattern = strdup(t);
                            }

                            if (add_regex(t, &ri->manpage_path_exclude) != 0) {
                                warn(_("error reading man page exclude path"));
                            }
                        }
                    } else if (group == BLOCK_XML) {
                        if (!strcmp(key, "include_path")) {
                            if (debug_mode) {
                                free(ri->xml_path_include_pattern);
                                ri->xml_path_include_pattern = strdup(t);
                            }

                            if (add_regex(t, &ri->xml_path_include) != 0) {
                                warn(_("error reading xml include path"));
                            }
                        } else if (!strcmp(key, "exclude_path")) {
                            if (debug_mode) {
                                free(ri->xml_path_exclude_pattern);
                                ri->xml_path_exclude_pattern = strdup(t);
                            }

                            if (add_regex(t, &ri->xml_path_exclude) != 0) {
                                warn(_("error reading xml exclude path"));
                            }
                        }
                    } else if (group == BLOCK_DESKTOP) {
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
                    } else if (group == BLOCK_FILESIZE) {
                        if (!strcmp(key, "size_threshold")) {
                            if (!strcasecmp(t, "info") || !strcasecmp(t, "info-only") || !strcasecmp(t, "info_only")) {
                                ri->size_threshold = -1;
                            } else {
                                ri->size_threshold = strtol(t, 0, 10);

                                if ((ri->size_threshold == LONG_MIN || ri->size_threshold == LONG_MAX) && errno == ERANGE) {
                                    warn("strtol()");
                                    ri->size_threshold = 0;
                                }
                            }
                        }
                    } else if (group == BLOCK_ANNOCHECK) {
                        process_table(key, t, &ri->annocheck);
                    } else if (group == BLOCK_JAVABYTECODE) {
                        process_table(key, t, &ri->jvm);
                    } else if (group == BLOCK_MIGRATED_PATHS) {
                        process_table(key, t, &ri->pathmigration);
                    } else if (group == BLOCK_PRODUCTS) {
                        process_table(key, t, &ri->products);
                    } else if (block == BLOCK_INSPECTIONS) {
                        if (!strcasecmp(t, "on")) {
                            exclude = false;
                        } else if (!strcasecmp(t, "off")) {
                            exclude = true;
                        } else {
                            exclude = false;
                            warnx(_("*** inspection flag must be 'on' or 'off', ignoring for '%s'"), key);
                        }

                        if (!process_inspection_flag(key, exclude, &ri->tests)) {
                            err(RI_PROGRAM_ERROR, _("*** Unknown inspection: `%s`"), key);
                        }
                    } else if (group == BLOCK_ABIDIFF) {
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
                        } else if (!strcmp(key, "kabi_dir")) {
                            free(ri->kabi_dir);
                            ri->kabi_dir = strdup(t);
                        } else if (!strcmp(key, "kabi_filename")) {
                            free(ri->kabi_filename);
                            ri->kabi_filename = strdup(t);
                        }
                    } else if (group == BLOCK_PATCHES) {
                        if (!strcmp(key, "file_count_threshold")) {
                            ri->patch_file_threshold = strtol(t, 0, 10);

                            if ((ri->patch_file_threshold == LONG_MIN || ri->patch_file_threshold == LONG_MAX) && errno == ERANGE) {
                                warn("strtol()");
                                ri->patch_file_threshold = DEFAULT_PATCH_FILE_THRESHOLD;
                            }
                        } else if (!strcmp(key, "line_count_threshold")) {
                            ri->patch_line_threshold = strtol(t, 0, 10);

                            if ((ri->patch_line_threshold == LONG_MIN || ri->patch_line_threshold == LONG_MAX) && errno == ERANGE) {
                                warn("strtol()");
                                ri->patch_line_threshold = DEFAULT_PATCH_LINE_THRESHOLD;
                            }
                        }
                    }
                } else if (symbol == SYMBOL_ENTRY) {
                    DEBUG_PRINT("    -> SYMBOL_ENTRY=%s, block=%d, group=%d\n", t, block, group);

                    if (block == BLOCK_BADFUNCS) {
                        add_entry(&ri->bad_functions, t);
                    } else if (block == BLOCK_BADWORDS) {
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
                    } else if (block == BLOCK_BIN_PATHS) {
                        add_entry(&ri->bin_paths, t);
                    } else if (block == BLOCK_FORBIDDEN_OWNERS) {
                        add_entry(&ri->forbidden_owners, t);
                    } else if (block == BLOCK_FORBIDDEN_GROUPS) {
                        add_entry(&ri->forbidden_groups, t);
                    } else if (block == BLOCK_IGNORE) {
                        /*
                         * BLOCK_NULL means we're reading the
                         * top-level 'ignore' block.  Any other group
                         * means we are reading the optional 'ignore'
                         * subblock for an inspection.
                         */
                        if (group == BLOCK_NULL) {
                            add_entry(&ri->ignores, t);
                        } else {
                            add_ignore(&ri->inspection_ignores, group, t);
                        }
                    } else if (block == BLOCK_SHELLS) {
                        add_entry(&ri->shells, t);
                    } else if (block == BLOCK_LTO_SYMBOL_NAME_PREFIXES) {
                        add_entry(&ri->lto_symbol_name_prefixes, t);
                    } else if (block == BLOCK_FORBIDDEN_PATHS) {
                        add_entry(&ri->forbidden_paths, t);
                    } else if (block == BLOCK_KERNEL_FILENAMES) {
                        add_entry(&ri->kernel_filenames, t);
                    } else if (block == BLOCK_PATCH_FILENAMES) {
                        add_entry(&ri->patch_ignore_list, t);
                    } else if (group == BLOCK_PATHMIGRATION_EXCLUDED_PATHS) {
                        add_entry(&ri->pathmigration_excluded_paths, t);
                    } else if (block == BLOCK_RUNPATH_ALLOWED_PATHS) {
                        add_entry(&ri->runpath_allowed_paths, t);
                    } else if (block == BLOCK_RUNPATH_ALLOWED_ORIGIN_PATHS) {
                        add_entry(&ri->runpath_allowed_origin_paths, t);
                    } else if (block == BLOCK_RUNPATH_ORIGIN_PREFIX_TRIM) {
                        add_entry(&ri->runpath_origin_prefix_trim, t);
                    } else if (block == BLOCK_EXPECTED_EMPTY_RPMS) {
                        add_entry(&ri->expected_empty_rpms, t);
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

    DEBUG_PRINT("YAML_STREAM_END_TOKEN\n");

    /* destroy the YAML parser, close the input file */
    yaml_parser_delete(&parser);

    if (fclose(fp) != 0) {
        warn("fclose()");
        return -1;
    }

    /* clean up */
    free(key);

    return 0;
}

/*
 * Initialize the fileinfo list for the given product release.  If the
 * file cannot be found, return false.
 */
bool init_fileinfo(struct rpminspect *ri)
{
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    char *line = NULL;
    char *token = NULL;
    char *fnpart = NULL;
    fileinfo_field_t field = MODE;
    fileinfo_entry_t *fientry = NULL;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->fileinfo) {
        return true;
    }

    /* the actual fileinfo file */
    xasprintf(&ri->fileinfo_filename, "%s/%s/%s", ri->vendor_data_dir, FILEINFO_DIR, ri->product_release);
    assert(ri->fileinfo_filename != NULL);
    contents = read_file(ri->fileinfo_filename);

    if (contents == NULL) {
        return false;
    }

    /* initialize the list */
    ri->fileinfo = calloc(1, sizeof(*(ri->fileinfo)));
    assert(ri->fileinfo != NULL);
    TAILQ_INIT(ri->fileinfo);

    /* add all the entries to the fileinfo list */
    TAILQ_FOREACH(entry, contents, items) {
        if (entry->data == NULL) {
            continue;
        }

        /* trim line ending characters */
        line = entry->data;
        line[strcspn(line, "\r\n")] = '\0';

        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r' || !strcmp(line, "")) {
            continue;
        }

        /* initialize a new list entry */
        fientry = calloc(1, sizeof(*fientry));
        assert(fientry != NULL);

        /* read the fields */
        while ((token = strsep(&line, " \t")) != NULL) {
            /* might be lots of space between fields */
            if (*token == '\0') {
                continue;
            }

            /* copy the field in to the correct struct member */
            if (field == MODE) {
                fientry->mode = parse_mode(token);
            } else if (field == OWNER) {
                fientry->owner = strdup(token);
            } else if (field == GROUP) {
                fientry->group = strdup(token);
            } else if (field == FILENAME) {
                fnpart = token;

                /* trim leading slashes since we compare to localpath later */
                while (*token != '/' && *token != '\0') {
                    token++;
                }

                if (*token == '\0') {
                    /* this is an invalid entry in the fileinfo list */
                    warnx(_("*** Invalid filename in the fileinfo list: %s"), fnpart);
                    warnx(_("*** From this invalid line:"));
                    warnx(_("***     %s"), line);

                    free(fientry->owner);
                    free(fientry->group);
                    free(fientry);
                    entry = NULL;
                } else {
                    fientry->filename = strdup(token);
                }

                break;     /* nothing should come after this field */
            }

            field++;
        }

        /* add the entry */
        if (fientry != NULL) {
            TAILQ_INSERT_TAIL(ri->fileinfo, fientry, items);
        }

        /* clean up */
        field = MODE;
    }

    list_free(contents, free);

    return true;
}

/*
 * Initialize the caps list for the given product release.  If the
 * file cannot be found, return false.
 */
bool init_caps(struct rpminspect *ri)
{
    char *line = NULL;
    char *token = NULL;
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    caps_field_t field = PACKAGE;
    caps_filelist_t *files = NULL;
    caps_entry_t *centry = NULL;
    caps_filelist_entry_t *filelist_entry = NULL;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->caps) {
        return true;
    }

    /* the actual caps list file */
    xasprintf(&ri->caps_filename, "%s/%s/%s", ri->vendor_data_dir, CAPABILITIES_DIR, ri->product_release);
    assert(ri->caps_filename != NULL);
    contents = read_file(ri->caps_filename);

    if (contents == NULL) {
        return false;
    }

    /* initialize the list */
    ri->caps = calloc(1, sizeof(*(ri->caps)));
    assert(ri->caps != NULL);
    TAILQ_INIT(ri->caps);

    /* add all the entries to the caps list */
    TAILQ_FOREACH(entry, contents, items) {
        if (entry->data == NULL) {
            continue;
        }

        /* trim line ending characters */
        line = entry->data;
        line[strcspn(line, "\r\n")] = '\0';

        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r' || !strcmp(line, "")) {
            continue;
        }

        /* read the fields */
        while ((token = strsep(&line, " \t")) != NULL) {
            /* might be lots of space between fields */
            if (*token == '\0') {
                continue;
            }

            /* take action on each field */
            if (field == PACKAGE) {
                /* this package may exist in the list */
                TAILQ_FOREACH(centry, ri->caps, items) {
                    if (!strcmp(centry->pkg, token)) {
                        files = centry->files;
                        break;
                    }
                }

                if (TAILQ_EMPTY(ri->caps) || files == NULL) {
                    /* new package for the list */
                    centry = calloc(1, sizeof(*centry));
                    assert(centry != NULL);
                    centry->pkg = strdup(token);
                    centry->files = calloc(1, sizeof(*centry->files));
                    assert(centry->files != NULL);
                    TAILQ_INIT(centry->files);
                    TAILQ_INSERT_TAIL(ri->caps, centry, items);
                    files = centry->files;
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
        files = NULL;
        field = PACKAGE;
    }

    list_free(contents, free);

    return true;
}

/*
 * Initialize the rebaseable list for the given product release and
 * cache it.  Return the cached list.  If the file cannot be found,
 * return false.
 */
bool init_rebaseable(struct rpminspect *ri)
{
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    char *line = NULL;
    string_entry_t *newentry = NULL;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->rebaseable) {
        return true;
    }

    /* the actual rebaseable list file */
    xasprintf(&ri->rebaseable_filename, "%s/%s/%s", ri->vendor_data_dir, REBASEABLE_DIR, ri->product_release);
    assert(ri->rebaseable_filename != NULL);
    contents = read_file(ri->rebaseable_filename);

    if (contents == NULL) {
        return false;
    }

    /* initialize the list */
    ri->rebaseable = calloc(1, sizeof(*(ri->rebaseable)));
    assert(ri->rebaseable != NULL);
    TAILQ_INIT(ri->rebaseable);

    /* add all the entries to the caps list */
    TAILQ_FOREACH(entry, contents, items) {
        if (entry->data == NULL) {
            continue;
        }

        /* trim line ending characters */
        line = entry->data;
        line[strcspn(line, "\r\n")] = '\0';

        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r' || !strcmp(line, "")) {
            continue;
        }

        /* add the entry to the actual list */
        newentry = calloc(1, sizeof(*newentry));
        assert(newentry != NULL);
        newentry->data = strdup(line);
        TAILQ_INSERT_TAIL(ri->rebaseable, newentry, items);
    }

    list_free(contents, free);

    return true;
}

/*
 * Initialize the politics list for the given product release.  If the
 * file cannot be found, return false.
 */
bool init_politics(struct rpminspect *ri)
{
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    char *line = NULL;
    char *token = NULL;
    politics_entry_t *pentry = NULL;
    politics_field_t field = PATTERN;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->politics) {
        return true;
    }

    /* the actual politics file */
    xasprintf(&ri->politics_filename, "%s/%s/%s", ri->vendor_data_dir, POLITICS_DIR, ri->product_release);
    assert(ri->politics_filename != NULL);
    contents = read_file(ri->politics_filename);

    if (contents == NULL) {
        return false;
    }

    /* initialize the list */
    ri->politics = calloc(1, sizeof(*(ri->politics)));
    assert(ri->politics != NULL);
    TAILQ_INIT(ri->politics);

    /* add all the entries to the politics list */
    TAILQ_FOREACH(entry, contents, items) {
        if (entry->data == NULL) {
            continue;
        }

        /* trim line ending characters */
        line = entry->data;
        line[strcspn(line, "\r\n")] = '\0';

        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r' || !strcmp(line, "")) {
            continue;
        }

        /* initialize a new list entry */
        pentry = calloc(1, sizeof(*pentry));
        assert(pentry != NULL);

        /* read the fields */
        while ((token = strsep(&line, " \t")) != NULL) {
            /* might be lots of space between fields */
            if (*token == '\0') {
                continue;
            }

            /* copy the field in to the correct struct member */
            if (field == PATTERN) {
                pentry->pattern = strdup(token);
            } else if (field == DIGEST) {
                pentry->digest = strdup(token);
            } else if (field == PERMISSION) {
                /*
                 * The permission column is either "allow" or "deny".
                 * The general assumption is that entries in this file
                 * will be for denial, so default to that and only do
                 * a case-insensitive search for 'allow' to allow the
                 * file. */
                if (!strcasecmp(token, "allow")) {
                    pentry->allowed = true;
                } else {
                    pentry->allowed = false;
                }

                break;     /* nothing should come after this field */
            }

            field++;
        }

        /* add the entry */
        if (pentry != NULL) {
            TAILQ_INSERT_TAIL(ri->politics, pentry, items);
        }

        /* clean up */
        field = PATTERN;
    }

    list_free(contents, free);

    return true;
}

/*
 * Initialize the security list for the given product release.  If the
 * file cannot be found, return false.
 */
bool init_security(struct rpminspect *ri)
{
    int pos = 0;
    char *line = NULL;
    char *token = NULL;
    char *path = NULL;
    char *pkg = NULL;
    char *ver = NULL;
    char *rel = NULL;
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    string_list_t *rules = NULL;
    string_entry_t *rentry = NULL;
    string_list_t *kv = NULL;
    string_entry_t *key = NULL;
    string_entry_t *value = NULL;
    int stype;
    enum secrule_action saction;
    security_entry_t *sentry = NULL;
    secrule_t *rule_entry = NULL;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->security_initialized) {
        return true;
    } else {
        ri->security = calloc(1, sizeof(*ri->security));
        assert(ri->security != NULL);
        TAILQ_INIT(ri->security);
    }

    /* the actual security file */
    xasprintf(&ri->security_filename, "%s/%s/%s", ri->vendor_data_dir, SECURITY_DIR, ri->product_release);
    assert(ri->security_filename != NULL);
    contents = read_file(ri->security_filename);

    if (contents == NULL) {
        return false;
    }

    /* add all the entries to the caps list */
    TAILQ_FOREACH(entry, contents, items) {
        if (entry->data == NULL) {
            continue;
        }

        /* trim line ending characters */
        line = entry->data;
        line[strcspn(line, "\r\n")] = '\0';

        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r' || !strcmp(line, "")) {
            continue;
        }

        /* read the fields */
        while ((token = strsep(&line, " \t")) != NULL) {
            /* might be lots of space between fields */
            if (*token == '\0') {
                continue;
            }

            if (pos == 0) {
                path = strdup(token);
                assert(path != NULL);
            } else if (pos == 1) {
                pkg = strdup(token);
                assert(pkg != NULL);
            } else if (pos == 2) {
                ver = strdup(token);
                assert(ver != NULL);
            } else if (pos == 3) {
                rel = strdup(token);
                assert(rel != NULL);
            } else if (pos == 4) {
                rules = strsplit(token, ",");
            }

            pos++;
        }

        /* add the entry */
        if (path && pkg && ver && rel && rules) {
            /* allocate a new entry */
            sentry = calloc(1, sizeof(*sentry));
            assert(sentry != NULL);

            /* the main values of a rule */
            free(sentry->pkg);
            sentry->pkg = strdup(pkg);
            free(sentry->ver);
            sentry->ver = strdup(ver);
            free(sentry->rel);
            sentry->rel = strdup(rel);

            /* the list of individual rules */
            TAILQ_FOREACH(rentry, rules, items) {
                /* split the RULE=ACTION setting */
                kv = strsplit(rentry->data, "=");

                /* we must have a rule and action */
                if (list_len(kv) != 2) {
                    warnx(_("*** invalid security rule: %s"), rentry->data);
                    list_free(kv, free);
                    continue;
                }

                key = TAILQ_FIRST(kv);
                value = TAILQ_LAST(kv, string_entry_s);

                /* find the rule */
                if (!strcasecmp(key->data, "caps")) {
                    stype = SECRULE_CAPS;
                } else if (!strcasecmp(key->data, "execstack")) {
                    stype = SECRULE_EXECSTACK;
                } else if (!strcasecmp(key->data, "relro")) {
                    stype = SECRULE_RELRO;
                } else if (!strcasecmp(key->data, "fortifysource")) {
                    stype = SECRULE_FORTIFYSOURCE;
                } else if (!strcasecmp(key->data, "pic")) {
                    stype = SECRULE_PIC;
                } else if (!strcasecmp(key->data, "textrel")) {
                    stype = SECRULE_TEXTREL;
                } else if (!strcasecmp(key->data, "setuid")) {
                    stype = SECRULE_SETUID;
                } else if (!strcasecmp(key->data, "worldwritable")) {
                    stype = SECRULE_WORLDWRITABLE;
                } else if (!strcasecmp(key->data, "securitypath")) {
                    stype = SECRULE_SECURITYPATH;
                } else if (!strcasecmp(key->data, "modes")) {
                    stype = SECRULE_MODES;
                } else {
                    stype = SECRULE_NULL;
                }

                if (stype == SECRULE_NULL) {
                    warnx(_("*** unknown security rule: %s"), key->data);
                    list_free(kv, free);
                    continue;
                }

                /* find the action */
                if (!strcasecmp(value->data, "skip")) {
                    saction = SECRULE_ACTION_SKIP;
                } else if (!strcasecmp(value->data, "inform")) {
                    saction = SECRULE_ACTION_INFORM;
                } else if (!strcasecmp(value->data, "verify")) {
                    saction = SECRULE_ACTION_VERIFY;
                } else if (!strcasecmp(value->data, "fail")) {
                    saction = SECRULE_ACTION_FAIL;
                } else {
                    saction = SECRULE_ACTION_NULL;
                }

                if (saction == SECRULE_ACTION_NULL) {
                    warnx(_("*** unknown security action: %s"), value->data);
                    list_free(kv, free);
                    continue;
                }

                /* add or replace the rule action */
                HASH_FIND_INT(sentry->rules, &stype, rule_entry);

                if (rule_entry == NULL) {
                    rule_entry = calloc(1, sizeof(*rule_entry));
                    assert(rule_entry != NULL);
                    rule_entry->type = stype;
                    rule_entry->action = saction;
                    HASH_ADD_INT(sentry->rules, type, rule_entry);
                } else {
                    /* rule already exists, just replace the action */
                    rule_entry->action = saction;
                }

                /* clean up */
                list_free(kv, free);
            }

            /* add new entry to the security rules list */
            TAILQ_INSERT_TAIL(ri->security, sentry, items);
        } else {
            warnx(_("*** malformed security line: %s"), line);
        }

        /* clean up */
        free(path);
        free(pkg);
        free(ver);
        free(rel);
        list_free(rules, free);
        pos = 0;
    }

    list_free(contents, free);
    ri->security_initialized = true;

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
        ri->patch_file_threshold = DEFAULT_PATCH_FILE_THRESHOLD;
        ri->patch_line_threshold = DEFAULT_PATCH_LINE_THRESHOLD;

        /* Initialize commands */
        ri->commands.diff = strdup(DIFF_CMD);
        ri->commands.diffstat = strdup(DIFFSTAT_CMD);
        ri->commands.msgunfmt = strdup(MSGUNFMT_CMD);
        ri->commands.desktop_file_validate = strdup(DESKTOP_FILE_VALIDATE_CMD);
        ri->commands.annocheck = strdup(ANNOCHECK_CMD);
        ri->commands.abidiff = strdup(ABIDIFF_CMD);
        ri->commands.kmidiff = strdup(KMIDIFF_CMD);

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
            warn(_("*** unable to read profile '%s' from %s\n"), profile, filename);
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

#if 0
    /* debugging output only to make sure we captured ignores */
    if (ri->ignores && !TAILQ_EMPTY(ri->ignores)) {
        string_entry_t *se = NULL;

        DEBUG_PRINT("global ignores:\n");

        TAILQ_FOREACH(se, ri->ignores, items) {
            DEBUG_PRINT("    - %s\n", se->data);
        }
    } else {
        DEBUG_PRINT("no global ignores defined\n");
    }

    if (ri->inspection_ignores) {
        string_list_map_t *e = NULL;
        string_list_map_t *te = NULL;
        string_entry_t *se = NULL;

        DEBUG_PRINT("per-inspection ignores:\n");

        HASH_ITER(hh, ri->inspection_ignores, e, te) {
            DEBUG_PRINT("    %s:\n", e->key);

            if (e->value && !TAILQ_EMPTY(e->value)) {
                TAILQ_FOREACH(se, e->value, items) {
                    DEBUG_PRINT("        - %s\n", se->data);
                }
            }
        }
    } else {
        DEBUG_PRINT("no per-inspection ignores defined\n");
    }
#endif

    return ri;
}
