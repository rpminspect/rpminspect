/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <err.h>
#include "callbacks.h"
#include "parser.h"
#include "rpminspect.h"
#include "queue.h"
#include "uthash.h"

/*
 * Debug printing on the config file parser is verbose, so it can be
 * disabled by commenting this line out and uncommented the
 * INIT_DEBUG_PRINT line that expands to void.
 */
/* #define INIT_DEBUG_PRINT DEBUG_PRINT */
#define INIT_DEBUG_PRINT(...) (void)(0)

/* List defaults (not in constants.h to avoid cpp shenanigans) */

/**
 * @def BIN_PATHS
 * NULL terminated array of strings of paths where executable files
 * may reside.
 */
static const char *BIN_PATHS[] = {"/bin", "/sbin", "/usr/bin", "/usr/sbin", NULL};

/**
 * @def SHELLS
 * NULL terminated array of strings of shells to use for syntax
 * checking (only the basename is needed).  All shells listed must
 * support the '-n' option for syntax checking.  The shell should exit
 * 0 if the syntax checker passes, non-zero otherwise.  The 'rc' shell
 * is an exception and has special handling in the 'shellsyntax'
 * inspection.
 */
static const char *SHELLS[] = {"sh", "ksh", "zsh", "csh", "tcsh", "rc", "bash", NULL};

/* Supported config filename endings (we assume type by this) */
static const char *CFG_FILENAME_EXTENSIONS[] = {"yaml", "json", "dson", NULL};

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

    *list = list_add(*list, s);
    return;
}

/*
 * Given an inspection identifier from the config file reader and a
 * list value, add it to the per-inspection list of ignores.
 */
static void add_string_list_map_entry(string_list_map_t **table, const char *key, const char *value)
{
    string_list_map_t *mapentry = NULL;

    assert(key != NULL);
    assert(value != NULL);

    /* look for the list first, add if not found */
    HASH_FIND_STR(*table, key, mapentry);

    if (mapentry == NULL) {
        /* start a new entry for this inspection */
        mapentry = calloc(1, sizeof(*mapentry));
        assert(mapentry != NULL);
        mapentry->key = strdup(key);
        mapentry->value = calloc(1, sizeof(*mapentry->value));
        assert(mapentry->value != NULL);
        TAILQ_INIT(mapentry->value);
        mapentry->value = list_add(mapentry->value, value);
        HASH_ADD_KEYPTR(hh, *table, mapentry->key, strlen(mapentry->key), mapentry);
    } else {
        /* on the off chance we have an empty list of values */
        if (mapentry->value == NULL) {
            mapentry->value = calloc(1, sizeof(*mapentry->value));
            assert(mapentry->value != NULL);
            TAILQ_INIT(mapentry->value);
        }

        /* add to existing ignore list */
        mapentry->value = list_add(mapentry->value, value);
    }

    return;
}

/**
 * Given a key and value, initialize a new hash table (clearing the
 * old one if necessary) and migrate the data from the list to the
 * hash table.  Clear the incoming hash table list.  The caller is
 * responsible for freeing all memory allocated to these structures.
 *
 * If required is set to true, this function will only append the
 * value to an existing key in the table.  Use this parameter if
 * processing a secondary table and you require the key to exist from
 * a previous processing run.
 *
 * NOTE: The incoming table entries will be removed and freed as the
 * data is migrated to the table and keys.  Before return, the
 * incoming table list will be freed.  The actual key and value
 * strings in each incoming table entry are not freed since they just
 * migrate over to the new structures, but the entries themselves are
 * removed and freed from the incoming table.
 *
 * @param key The key in the hash table (string).
 * @param value The value to add to the hash table (string).
 * @param required Set to true if key must exist to append value to list.
 * @param single Set to true if key must contain only a single value,
 *               otherwise it's a string list.
 * @param table Destination hash table.
 */
static void process_table(const char *key, const char *value, const bool required, const bool single, string_map_t **table)
{
    char *tmp = NULL;
    string_list_t *tokens = NULL;
    string_map_t *entry = NULL;

    assert(key != NULL);
    assert(value != NULL);

    /* look for the key first */
    HASH_FIND_STR(*table, key, entry);

    if (entry == NULL) {
        if (required) {
            warnx("missing key `%s', unable to append `%s'", key, value);
        } else {
            /* add the new key/value pair to the table */
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->key = strdup(key);
            entry->value = strdup(value);
            HASH_ADD_KEYPTR(hh, *table, entry->key, strlen(entry->key), entry);
        }
    } else {
        /* entry found, handle the value */
        if (entry->value != NULL && !single) {
            tokens = strsplit(entry->value, " ");
            assert(tokens != NULL);

            tokens = list_add(tokens, value);

            tmp = list_to_string(tokens, " ");
            list_free(tokens, free);
        } else {
            tmp = strdup(value);
        }

        assert(tmp != NULL);
        free(entry->value);
        entry->value = tmp;
    }

    return;
}

/*
 * Convert a 10 character mode string for a file to a mode_t
 * For example, convert "-rwsr-xr-x" to a mode_t
 */
static mode_t parse_mode(const char *input)
{
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
 * Retrieve a string from configuration, updating the specified field if it
 * has already been defined.
 */
static inline void strget(parser_plugin *p, parser_context *ctx, const char *key1, const char *key2, char **dest)
{
    char *res;

    res = p->getstr(ctx, key1, key2);

    if (res == NULL) {
        return;
    }

    free(*dest);
    *dest = res;
    return;
}

/* lambda for array() below. */
static bool array_cb(const char *entry, void *cb_data)
{
    string_list_t **list = cb_data;
    add_entry(list, entry);
    return false;
}

/* Transform configuration's array into a string_list_t. */
static inline void array(parser_plugin *p, parser_context *ctx, const char *key1, const char *key2, string_list_t **list)
{
    if (p->strarray_foreach(ctx, key1, key2, array_cb, list)) {
        warnx(_("problem adding entries to array %s->%s"), key1, key2);
    }

    return;
}

/* lambda for add_ignores() below. */
static bool add_ignores_cb(const char *entry, void *cb_data)
{
    add_ignores_cb_data *d = cb_data;
    add_string_list_map_entry(d->inspection_ignores, d->inspection, entry);
    return false;
}

/* Load and add an inspection's ignores section. */
static inline void add_ignores(struct rpminspect *ri, parser_plugin *p, parser_context *ctx, const char *inspection)
{
    add_ignores_cb_data data = { &ri->inspection_ignores, inspection };

    if (p->strarray_foreach(ctx, inspection, "ignore", add_ignores_cb, &data)
        && p->strarray_foreach(ctx, inspection, "ignores", add_ignores_cb, &data)) {
        warnx(_("problem adding ignore entries to %s"), inspection);
    }

    return;
}

/*
 * Handle inclusion and exclusion paths for an inspection.  Pass the bare name
 * of an inspection, not a string.  Reuses variables from context.
 */
#define ADD_INCL_EXCL(inspection)                                       \
    s = p->getstr(ctx, #inspection, "include_path");                    \
                                                                        \
    if (s != NULL) {                                                    \
        if (debug_mode) {                                               \
            ri->inspection ## _path_include_pattern = strdup(s);        \
        }                                                               \
                                                                        \
        if (add_regex(s, &ri->inspection ## _path_include) != 0) {      \
            warn(_("error reading " #inspection " include path"));      \
        }                                                               \
                                                                        \
        free(s);                                                        \
    }                                                                   \
                                                                        \
    s = p->getstr(ctx, #inspection, "exclude_path");                    \
                                                                        \
    if (s != NULL) {                                                    \
        if (debug_mode) {                                               \
            ri->inspection ## _path_exclude_pattern = strdup(s);        \
        }                                                               \
                                                                        \
        if (add_regex(s, &ri->inspection ## _path_exclude) != 0) {      \
            warn(_("error reading " #inspection " include path"));      \
        }                                                               \
                                                                        \
        free(s);                                                        \
    }

/* lambda for tabledict below. */
static bool tabledict_cb(const char *key, const char *value, void *cb_data)
{
    tabledict_cb_data *data = cb_data;

    /* javabytecode uses this at top-level, but also supports ignores. */
    if (!strcasecmp(key, "ignore")) {
        return false;
    }

    process_table(key, value, data->required, data->single, data->table);
    return false;
}

/* Process a dictionary of strings into the specified table. */
static void tabledict(parser_plugin *p, parser_context *ctx, const char *key1, const char *key2, string_map_t **table, bool required, bool single)
{
    tabledict_cb_data data = { required, single, table };

    if (p->strdict_foreach(ctx, key1, key2, tabledict_cb, &data)) {
        warnx(_("ignoring malformed section %s->%s"), key1, key2);
    }

    return;
}

/* lambda for handling old-style top-level annocheck jobs.  A special case of
 * tabledict_cb that skips more keys. */
static bool annocheck_cb(const char *key, const char *value, void *cb_data)
{
    tabledict_cb_data *data = cb_data;

    if (!strcasecmp(key, "failure_severity") || !strcasecmp(key, "extra_opts") || !strcasecmp(key, "ignore")) {
        return false;
    }

    process_table(key, value, data->required, data->single, data->table);
    return false;
}

/* lambda to handle entries in the rpmdeps config file section. */
static bool rpmdeps_cb(const char *key, const char *value, void *cb_data)
{
    deprule_ignore_map_t **deprules_ignore = cb_data;
    dep_type_t depkey = TYPE_NULL;
    deprule_ignore_map_t *drentry = NULL;

    if (!strcmp(key, "requires")) {
        depkey = TYPE_REQUIRES;
    } else if (!strcmp(key, "provides")) {
        depkey = TYPE_PROVIDES;
    } else if (!strcmp(key, "conflicts")) {
        depkey = TYPE_CONFLICTS;
    } else if (!strcmp(key, "obsoletes")) {
        depkey = TYPE_OBSOLETES;
    } else if (!strcmp(key, "enhances")) {
        depkey = TYPE_ENHANCES;
    } else if (!strcmp(key, "recommends")) {
        depkey = TYPE_RECOMMENDS;
    } else if (!strcmp(key, "suggests")) {
        depkey = TYPE_SUGGESTS;
    } else if (!strcmp(key, "supplements")) {
        depkey = TYPE_SUPPLEMENTS;
    }

    if (depkey != TYPE_NULL) {
        HASH_FIND_INT(*deprules_ignore, &depkey, drentry);
    }

    /* overwrite existing entry, otherwise create new one */
    if (drentry == NULL) {
        drentry = calloc(1, sizeof(*drentry));
        assert(drentry != NULL);
        drentry->type = depkey;
        drentry->pattern = strdup(value);

        if (add_regex(value, &drentry->ignore) != 0) {
            warn(_("error reading %s ignore pattern"), get_deprule_desc(depkey));
        }

        HASH_ADD_INT(*deprules_ignore, type, drentry);
    } else {
        free(drentry->pattern);
        drentry->pattern = NULL;
        drentry->type = depkey;
        drentry->pattern = strdup(value);
        regfree(drentry->ignore);
        drentry->ignore = NULL;

        if (add_regex(value, &drentry->ignore) != 0) {
            warn(_("error reading %s ignore pattern"), get_deprule_desc(depkey));
        }
    }

    return false;
}

/* lambda for handle_inspections() below. */
static bool handle_inspections_cb(const char *key, const char *value, void *cb_data)
{
    uint64_t *tests = cb_data;
    bool onoff = true;

    if (!strcasecmp(value, "on")) {
        onoff = false;
    } else if (strcasecmp(value, "off")) {
        warnx(_("*** flag must be 'on' or 'off'; ignoring '%s'"), value);
    }

    if (!process_inspection_flag(key, onoff, tests)) {
        err(RI_PROGRAM_ERROR, _("*** Unknown inspections: `%s`"), key);
        return true;
    }

    return false;
}

/* Turn inspections on and off from the "inspections" config section. */
static inline void handle_inspections(struct rpminspect *ri, parser_plugin *p, parser_context *ctx)
{
    if (p->strdict_foreach(ctx, "inspections", NULL, handle_inspections_cb, &ri->tests)) {
        warnx(_("malformed/unknown inspections section"));
    }

    return;
}

/* lambda for adding entries from the badfuncs_allowed configuration. */
static bool badfuncs_allowed_cb(const char *key, const char *value, void *cb_data)
{
    string_list_map_t **bad_functions_allowed = cb_data;

    add_string_list_map_entry(bad_functions_allowed, key, value);
    return false;
}

/*
 * Read either the main configuration file or a configuration file
 * overlay (profile) and populate the struct rpminspect members.
 */
static void read_cfgfile(struct rpminspect *ri, const char *filename)
{
    parser_plugin *p = NULL;
    parser_context *ctx = NULL;
    char *s = NULL;
    tabledict_cb_data annocheck_cb_data = { false, false, &ri->annocheck };

    assert(ri != NULL);
    assert(filename != NULL);

    INIT_DEBUG_PRINT("filename=|%s|\n", filename);

    if (parse_agnostic(filename, &p, &ctx)) {
        warnx(_("ignoring malformed %s configuration file: %s"), COMMAND_NAME, filename);
        return;
    }

    /* Processing order doesn't matter, so match data/generic.yaml. */
    strget(p, ctx, "common", "workdir", &ri->workdir);
    strget(p, ctx, "common", "profiledir", &ri->profiledir);
    strget(p, ctx, "koji", "hub", &ri->kojihub);
    strget(p, ctx, "koji", "download_ursine", &ri->kojiursine);
    strget(p, ctx, "koji", "download_mbs", &ri->kojimbs);
    strget(p, ctx, "commands", "msgunfmt", &ri->commands.msgunfmt);
    strget(p, ctx, "commands", "desktop-file-validate", &ri->commands.desktop_file_validate);
    strget(p, ctx, "commands", "abidiff", &ri->commands.abidiff);
    strget(p, ctx, "commands", "kmidiff", &ri->commands.kmidiff);
    strget(p, ctx, "vendor", "vendor_data_dir", &ri->vendor_data_dir);
    array(p, ctx, "vendor", "licensedb", &ri->licensedb);

    s = p->getstr(ctx, "environment", "product_release");

    if (s != NULL) {
        ri->have_environment = true;
        free(ri->product_release);
        ri->product_release = s;
        s = NULL;
    }

    s = p->getstr(ctx, "vendor", "favor_release");

    if (s != NULL) {
        if (!strcasecmp(s, "oldest")) {
            ri->favor_release = FAVOR_OLDEST;
        } else if (!strcasecmp(s, "newest")) {
            ri->favor_release = FAVOR_NEWEST;
        } else {
            ri->favor_release = FAVOR_NONE;
        }

        free(s);
    }

    handle_inspections(ri, p, ctx);
    tabledict(p, ctx, "products", NULL, &ri->products, false, false);
    array(p, ctx, "macrofiles", NULL, &ri->macrofiles);
    array(p, ctx, "ignore", NULL, &ri->ignores);
    array(p, ctx, "security_path_prefix", NULL, &ri->security_path_prefix);
    array(p, ctx, "badwords", NULL, &ri->badwords);
    strget(p, ctx, "metadata", "vendor", &ri->vendor);
    array(p, ctx, "metadata", "buildhost_subdomain", &ri->buildhost_subdomain);

#ifdef _HAVE_MODULARITYLABEL
    s = p->getstr(ctx, "modularity", "static_context");

    if (s != NULL) {
        if (!strcasecmp(s, "required")) {
            ri->modularity_static_context = STATIC_CONTEXT_REQUIRED;
        } else if (!strcasecmp(s, "forbidden")) {
            ri->modularity_static_context = STATIC_CONTEXT_FORBIDDEN;
        } else if (!strcasecmp(s, "recommend")) {
            ri->modularity_static_context = STATIC_CONTEXT_RECOMMEND;
        } else {
            ri->modularity_static_context = STATIC_CONTEXT_NULL;
            warnx(_("*** unknown modularity static context settiongs '%s'"), s);
        }

        free(s);
    }
#endif

    ADD_INCL_EXCL(elf);
    add_ignores(ri, p, ctx, "elf");
    array(p, ctx, "emptyrpm", "expected_empty", &ri->expected_empty_rpms);
    ADD_INCL_EXCL(manpage);
    add_ignores(ri, p, ctx, "manpage");
    ADD_INCL_EXCL(xml);
    add_ignores(ri, p, ctx, "xml");
    strget(p, ctx, "desktop", "desktop_entry_files_dir", &ri->desktop_entry_files_dir);
    add_ignores(ri, p, ctx, "desktop");
    array(p, ctx, "changedfiles", "header_file_extensions", &ri->header_file_extensions);
    add_ignores(ri, p, ctx, "changedfiles");
    array(p, ctx, "addedfiles", "forbidden_path_prefixes", &ri->forbidden_path_prefixes);
    array(p, ctx, "addedfiles", "forbidden_path_suffixes", &ri->forbidden_path_suffixes);
    array(p, ctx, "addedfiles", "forbidden_directories", &ri->forbidden_directories);
    add_ignores(ri, p, ctx, "addedfiles");
    add_ignores(ri, p, ctx, "movedfiles");
    add_ignores(ri, p, ctx, "removedfiles");
    array(p, ctx, "ownership", "bin_paths", &ri->bin_paths);
    strget(p, ctx, "ownership", "bin_owner", &ri->bin_owner);
    strget(p, ctx, "ownership", "bin-group", &ri->bin_group);
    array(p, ctx, "ownership", "forbidden_owners", &ri->forbidden_owners);
    array(p, ctx, "ownership", "forbidden_groups", &ri->forbidden_groups);
    add_ignores(ri, p, ctx, "ownership");
    array(p, ctx, "shellsyntax", "shells", &ri->shells);
    add_ignores(ri, p, ctx, "shellsyntax");

    s = p->getstr(ctx, "filesize", "size_threshold");

    if (s != NULL) {
        if (!strcasecmp(s, "info") || !strcasecmp(s, "info-only") || !strcasecmp(s, "info_only")) {
            ri->size_threshold = -1;
        } else {
            ri->size_threshold = strtol(s, 0, 10);

            if ((ri->size_threshold == LONG_MIN || ri->size_threshold == LONG_MAX) && errno == ERANGE) {
                warn("strtol");
                ri->size_threshold = 0;
            }
        }

        free(s);
    }

    add_ignores(ri, p, ctx, "filesize");
    array(p, ctx, "lto", "lto_symbol_name_prefixes", &ri->lto_symbol_name_prefixes);
    add_ignores(ri, p, ctx, "lto");

    s = p->getstr(ctx, "specname", "match");

    if (s != NULL) {
        if (!strcasecmp(s, "suffix")) {
            ri->specmatch = MATCH_SUFFIX;
        } else if (!strcasecmp(s, "prefix")) {
            ri->specmatch = MATCH_PREFIX;
        } else if (!strcasecmp(s, "full")) {
            ri->specmatch = MATCH_FULL;
        } else {
            warnx(_("*** unknown specname match setting '%s', defaulting to 'full'"), s);
            ri->specmatch = MATCH_FULL;
        }

        free(s);
    }

    s = p->getstr(ctx, "specname", "primary");

    if (s != NULL) {
        if (!strcasecmp(s, "filename")) {
            ri->specprimary = PRIMARY_FILENAME;
        } else if (!strcasecmp(s, "name")) {
            ri->specprimary = PRIMARY_NAME;
        } else {
            warnx(_("*** unknown specname primary setting '%s', defaulting to 'name'"), s);
            ri->specprimary = PRIMARY_NAME;
        }

        free(s);
    }

    add_ignores(ri, p, ctx, "specname");

    s = p->getstr(ctx, "annocheck", "failure_severity");

    if (s != NULL) {
        ri->annocheck_failure_severity = getseverity(s, RESULT_NULL);

        if (ri->annocheck_failure_severity == RESULT_NULL) {
            warnx(_("Invalid annocheck failure_reporting_level: %s, defaulting to %s."), s, strseverity(RESULT_VERIFY));
            ri->annocheck_failure_severity = RESULT_VERIFY;
        }

        free(s);
    }

    strget(p, ctx, "annocheck", "profile", &ri->annocheck_profile);
    tabledict(p, ctx, "annocheck", "jobs", &ri->annocheck, false, false);
    tabledict(p, ctx, "annocheck", "extra_opts", &ri->annocheck, true, false);
    add_ignores(ri, p, ctx, "annocheck");

    /* Backward compatibility for annocheck jobs at top-level. */
    if (ri->annocheck == NULL) {
        p->strdict_foreach(ctx, "annocheck", NULL, annocheck_cb, &annocheck_cb_data);
    }

    tabledict(p, ctx, "javabytecode", NULL, &ri->jvm, false, true);
    add_ignores(ri, p, ctx, "javabytecode");
    tabledict(p, ctx, "pathmigration", "migrated_paths", &ri->pathmigration, false, false);
    array(p, ctx, "pathmigration", "excluded_paths", &ri->pathmigration_excluded_paths);
    add_ignores(ri, p, ctx, "pathmigraion");
    add_ignores(ri, p, ctx, "politics");
    array(p, ctx, "files", "forbidden_paths", &ri->forbidden_paths);
    add_ignores(ri, p, ctx, "files");
    strget(p, ctx, "abidiff", "suppression_file", &ri->abidiff_suppression_file);
    strget(p, ctx, "abidiff", "debuginfo_path", &ri->abidiff_debuginfo_path);
    strget(p, ctx, "abidiff", "extra_args", &ri->abidiff_extra_args);

    s = p->getstr(ctx, "abdiff", "security_level_threshold");

    if (s != NULL) {
        ri->abi_security_threshold = strtol(s, 0, 10);

        if ((ri->abi_security_threshold == LONG_MIN ||
             ri->abi_security_threshold == LONG_MAX) && errno == ERANGE) {
            warn("strtol");
            ri->abi_security_threshold = DEFAULT_ABI_SECURITY_THRESHOLD;
        }

        free(s);
    }

    add_ignores(ri, p, ctx, "abidiff");
    strget(p, ctx, "kmidiff", "suppression_file", &ri->kmidiff_suppression_file);
    strget(p, ctx, "kmidiff", "debuginfo_path", &ri->kmidiff_debuginfo_path);
    strget(p, ctx, "kmidiff", "extra_args", &ri->kmidiff_extra_args);
    array(p, ctx, "kmidiff", "kernel_filenames", &ri->kernel_filenames);
    strget(p, ctx, "kmidiff", "kabi_dir", &ri->kabi_dir);
    strget(p, ctx, "kmidiff", "kabi_filename", &ri->kabi_filename);
    add_ignores(ri, p, ctx, "kmidiff");
    array(p, ctx, "patches", "automacros", &ri->automacros);
    array(p, ctx, "patches", "ignore_list", &ri->patch_ignore_list);
    array(p, ctx, "badfuncs", "forbidden", &ri->bad_functions);

    /* ri->bad_functions_allowed is a string_list_map_t, not string_map_t. */
    if (p->strdict_foreach(ctx, "badfuncs", "allowed", badfuncs_allowed_cb, &ri->bad_functions_allowed)) {
        warnx(_("Malformed badfuncs->allowed section"));
    }

    /* Backward compatibility for badfuncs prior to "forbidden". */
    if (ri->bad_functions == NULL && ri->bad_functions_allowed == NULL) {
        array(p, ctx, "badfuncs", NULL, &ri->bad_functions);
    }

    add_ignores(ri, p, ctx, "badfuncs");
    array(p, ctx, "runpath", "allowed_paths", &ri->runpath_allowed_paths);
    array(p, ctx, "runpath", "allowed_origin_paths", &ri->runpath_allowed_origin_paths);
    array(p, ctx, "runpath", "origin_prefix_trim", &ri->runpath_origin_prefix_trim);
    add_ignores(ri, p, ctx, "runpath");
    add_ignores(ri, p, ctx, "types");

    s = p->getstr(ctx, "unicode", "exclude");

    if (s != NULL && add_regex(s, &ri->unicode_exclude) != 0) {
        warn(_("error reading unicode exclude regular expression"));
    }

    free(s);

    array(p, ctx, "unicode", "excluded_mime_types", &ri->unicode_excluded_mime_types);
    array(p, ctx, "unicode", "forbidden_codepoints", &ri->unicode_forbidden_codepoints);
    add_ignores(ri, p, ctx, "unicode");

    if (p->strdict_foreach(ctx, "rpmdeps", "ignore", rpmdeps_cb, &ri->deprules_ignore)) {
        warnx(_("Malformed rpmdeps->ignore section; skipping"));
    }

    add_ignores(ri, p, ctx, "virus");
    add_ignores(ri, p, ctx, "capabilities");
    add_ignores(ri, p, ctx, "config");
    add_ignores(ri, p, ctx, "doc");
    add_ignores(ri, p, ctx, "kmod");
    add_ignores(ri, p, ctx, "permissions");
    add_ignores(ri, p, ctx, "symlinks");
    add_ignores(ri, p, ctx, "upstream");
    add_ignores(ri, p, ctx, "debuginfo");
    strget(p, ctx, "debuginfo", "debuginfo_sections", &ri->debuginfo_sections);

    p->fini(ctx);
    return;
}

/*
 * Given a directory and a filename with an extension, try to look for
 * it under all support filename extensions.  If found, return the
 * full path to the config file to the caller.  The caller is
 * responsible for freeing this string.  NULL is return if no config
 * file is found.
 */
static char *find_cfgfile(const char *dir, const char *name)
{
    int i = 0;
    char *tmp = NULL;
    char *filename = NULL;

    assert(dir != NULL);
    assert(name != NULL);

    for (i = 0; CFG_FILENAME_EXTENSIONS[i] != NULL; i++) {
        xasprintf(&tmp, "%s/%s.%s", dir, name, CFG_FILENAME_EXTENSIONS[i]);
        filename = realpath(tmp, NULL);

        if (filename && !access(filename, F_OK|R_OK)) {
            free(tmp);
            return filename;
        }

        free(tmp);
        free(filename);
    }

    return NULL;
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
    if (ri->fileinfo_filename == NULL) {
        xasprintf(&ri->fileinfo_filename, "%s/%s/%s", ri->vendor_data_dir, FILEINFO_DIR, ri->product_release);
        assert(ri->fileinfo_filename != NULL);
    }

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
#ifdef _WITH_LIBCAP
bool init_caps(struct rpminspect *ri)
{
    char *line = NULL;
    char *token = NULL;
    char *delim = NULL;
    char *end = NULL;
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
    if (ri->caps_filename == NULL) {
        xasprintf(&ri->caps_filename, "%s/%s/%s", ri->vendor_data_dir, CAPABILITIES_DIR, ri->product_release);
        assert(ri->caps_filename != NULL);
    }

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

        /*
         * the first two fields are space delimited, but then take the
         * 3rd field to the end of the line
         */
        delim = " \t";

        /* read the fields */
        while ((token = strsep(&line, delim)) != NULL) {
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
                field = FILEPATH;
            } else if (field == FILEPATH && filelist_entry->path == NULL) {
                filelist_entry->path = strdup(token);
                field = CAPABILITIES;

                /* reset to take the remaining part of the line as the 3rd field */
                delim = "\r\n";
            } else if (field == CAPABILITIES && filelist_entry->caps == NULL) {
                /* trim leading whitespace */
                while (isspace(*token) && *token != '\0') {
                    token++;
                }

                /* trim trailing whitespace */
                end = token;
                end += strlen(token);

                while (isspace(*end) && end != token) {
                    *end = '\0';
                    end--;
                }

                filelist_entry->caps = strdup(token);
                assert(filelist_entry->caps != NULL);
            } else {
                errx(RI_PROGRAM_ERROR, _("*** unexpected token `%s' seen in %s, cannot continue"), token, ri->caps_filename);
            }
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
#endif

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

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->rebaseable) {
        return true;
    }

    /* the actual rebaseable list file */
    if (ri->rebaseable_filename == NULL) {
        xasprintf(&ri->rebaseable_filename, "%s/%s/%s", ri->vendor_data_dir, REBASEABLE_DIR, ri->product_release);
        assert(ri->rebaseable_filename != NULL);
    }

    contents = read_file(ri->rebaseable_filename);

    if (contents == NULL) {
        return false;
    }

    /* initialize the list */
    ri->rebaseable = calloc(1, sizeof(*(ri->rebaseable)));
    assert(ri->rebaseable != NULL);
    TAILQ_INIT(ri->rebaseable);

    /* add all the entries to the rebaseable list */
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
        ri->rebaseable = list_add(ri->rebaseable, line);
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
    secrule_type_t stype = SECRULE_NULL;
    severity_t severity = RESULT_NULL;
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
        ri->security_initialized = true;
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
            free(sentry->path);
            sentry->path = strdup(path);
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
                stype = get_secrule_type(key->data);

                if (stype == SECRULE_NULL) {
                    warnx(_("*** unknown security rule: %s"), key->data);
                    list_free(kv, free);
                    continue;
                }

                /* find the action */
                severity = get_secrule_severity(value->data);

                if (severity == RESULT_NULL) {
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
                    rule_entry->severity = severity;
                    HASH_ADD_INT(sentry->rules, type, rule_entry);
                } else {
                    /* rule already exists, just replace the action */
                    rule_entry->severity = severity;
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

    return true;
}

/*
 * Initialize the icons list for the given product release and cache
 * it.  Return the cached list.  If the file cannot be found, return
 * false.
 */
bool init_icons(struct rpminspect *ri)
{
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    char *line = NULL;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->icons) {
        return true;
    }

    /* the actual icons list file */
    xasprintf(&ri->icons_filename, "%s/%s/%s", ri->vendor_data_dir, ICONS_DIR, ri->product_release);
    assert(ri->icons_filename != NULL);
    contents = read_file(ri->icons_filename);

    if (contents == NULL) {
        return false;
    }

    /* initialize the list */
    ri->icons = calloc(1, sizeof(*(ri->icons)));
    assert(ri->icons != NULL);
    TAILQ_INIT(ri->icons);

    /* add all the entries to the icons list */
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
        ri->icons = list_add(ri->icons, line);
    }

    list_free(contents, free);

    return true;
}

/*
 * Memory initialization function for struct rpminspect.
 */
struct rpminspect *calloc_rpminspect(struct rpminspect *ri)
{
    if (ri != NULL) {
        return ri;
    }

    /* Only initialize if we were given NULL for ri */
    ri = calloc(1, sizeof(*ri));
    assert(ri != NULL);

    /* Initialize the struct before reading files */
    ri->workdir = strdup(DEFAULT_WORKDIR);
    ri->vendor_data_dir = strdup(VENDOR_DATA_DIR);
    ri->favor_release = FAVOR_NEWEST;
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
    ri->abi_security_threshold = DEFAULT_ABI_SECURITY_THRESHOLD;
    ri->kmidiff_suppression_file = strdup(ABI_SUPPRESSION_FILE);
    ri->kmidiff_debuginfo_path = strdup(DEBUG_PATH);
    ri->annocheck_failure_severity = RESULT_VERIFY;
    ri->size_threshold = -1;
    ri->debuginfo_sections = strdup(ELF_SYMTAB" "ELF_DEBUG_INFO);

    /* Initialize commands */
    ri->commands.msgunfmt = strdup(MSGUNFMT_CMD);
    ri->commands.desktop_file_validate = strdup(DESKTOP_FILE_VALIDATE_CMD);
    ri->commands.abidiff = strdup(ABIDIFF_CMD);
    ri->commands.kmidiff = strdup(KMIDIFF_CMD);
#ifdef _WITH_ANNOCHECK
    ri->commands.annocheck = strdup(ANNOCHECK_CMD);
#endif

    /* Store full paths to all config files read */
    if (ri->cfgfiles == NULL) {
        ri->cfgfiles = calloc(1, sizeof(*ri->cfgfiles));
        assert(ri->cfgfiles != NULL);
        TAILQ_INIT(ri->cfgfiles);
    }

    return ri;
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
    char cwd[PATH_MAX + 1];
    char *tmp = NULL;
    char *cf = NULL;
    char *bn = NULL;
    char *kernelnames[] = KERNEL_FILENAMES;
    string_entry_t *cfg = NULL;

    if (ri == NULL) {
        ri = calloc_rpminspect(ri);
    }

    /* Read in the config file */
    if (cfgfile) {
        cfg = calloc(1, sizeof(*cfg));
        assert(cfg != NULL);
        cfg->data = realpath(cfgfile, NULL);

        /* In case we have a missing configuration file */
        if ((cfg->data == NULL) || (access(cfg->data, F_OK|R_OK) == -1)) {
            free(cfg->data);
            free(cfg);
            errx(RI_PROGRAM_ERROR, _("*** missing configuration file `%s'"), cfgfile);
        }

        /* Read the main configuration file to get things started */
        read_cfgfile(ri, cfg->data);

        /* Store this config file as one we read in */
        if (!list_contains(ri->cfgfiles, cfg->data)) {
            TAILQ_INSERT_TAIL(ri->cfgfiles, cfg, items);
        } else {
            free(cfg->data);
            free(cfg);
        }
    }

    /* Look for and autoload a product release profile if we have one */
    if (ri->product_release) {
        xasprintf(&tmp, "%s/%s", ri->profiledir, PRODUCT_RELEASE_CFGFILE_SUBDIR);
        assert(tmp != NULL);
        cf = find_cfgfile(tmp, ri->product_release);
        free(tmp);

        if (cf) {
            read_cfgfile(ri, cf);
            free(cf);
        }
    }

    /* If a profile is specified, read an overlay config file */
    if (profile) {
        cf = find_cfgfile(ri->profiledir, profile);

        if (cf) {
            read_cfgfile(ri, cf);
            free(cf);
        } else {
            errx(RI_MISSING_PROFILE, _("*** unable to find profile '%s'"), profile);
        }
    }

    /* get current dir */
    memset(cwd, '\0', sizeof(cwd));

    if (getcwd(cwd, PATH_MAX) == NULL) {
        err(RI_PROGRAM_ERROR, "getcwd");
    }

    /* optional config file from current directory */
    cf = find_cfgfile(cwd, COMMAND_NAME);

    if (cf) {
        read_cfgfile(ri, cf);

        /* save a copy of the local rpminspect config file for diagnostics */
        bn = basename(cf);
        assert(bn != NULL);
        ri->localcfg = strdup(bn);
        assert(ri->localcfg != NULL);
        ri->locallines = read_file(cf);
        assert(ri->locallines != NULL);

        free(cf);
    }

    /* Initialize some lists if we did not get any config file data */
    if (ri->kernel_filenames == NULL) {
        ri->kernel_filenames = calloc(1, sizeof(*ri->kernel_filenames));
        assert(ri->kernel_filenames != NULL);
        TAILQ_INIT(ri->kernel_filenames);

        for(i = 0; kernelnames[i] != NULL; i++) {
            ri->kernel_filenames = list_add(ri->kernel_filenames, kernelnames[i]);
        }
    }

    /* the rest of the members are used at runtime */
    ri->threshold = RESULT_VERIFY;
    ri->worst_result = RESULT_OK;
    ri->suppress = RESULT_NULL;

    if (ri->peers == NULL) {
        ri->peers = init_peers();
    }

    return ri;
}
