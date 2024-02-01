/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/*
 * This header defines a pluggable interface for structured language parsers
 *
 * Boolean-returning functions return true on error (and cease iteration), and
 * false otherwise.
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _LIBRPMINSPECT_PARSER_H
#define _LIBRPMINSPECT_PARSER_H

#include <stdbool.h>
#include <string.h>

/* Abstract type - implementations should cast to an appropriate type. */
typedef struct parser_context_st *parser_context;

/* Initialize a parser context for the given file. */
typedef bool (*parser_parse_file_fn)(parser_context **countext_out, const char *filepath);

/* Deinitialize/deallocate a parser context. */
typedef void (*parser_fini_fn)(parser_context *context);

/* Returns true if the named section is present. */
typedef bool (*parser_have_section_fn)(parser_context *context, const char *section);

/*
 * Return a copy of the string at the specified position in the parsed
 * structure.  All structures are dictionary-like.  Pass NULL for the second
 * key if there's only one level of nesting, and NULL for both keys to operate
 * on the top-level structure.  free() the returned string when done.  Returns
 * NULL if string not found or the object at the specified position wasn't a
 * string.
 */
typedef char *(*parser_getstr_fn)(parser_context *context, const char *key1, const char *key2);

/* Map across arrays of stringlike objects.  key1, key2 as in getstr. */
typedef bool (*parser_strarray_entry_fn)(const char *entry, void *cb_data);
typedef bool (*parser_strarray_foreach_fn)(parser_context *context, const char *key1, const char *key2, parser_strarray_entry_fn lambda, void *cb_data);

/*
 * Map across dictionaries of stringlike objects.  While keymap() can be used
 * instead, strdict_foreach() avoids a layer of indirecton for dictionaries
 * where values are strings.  key1, key2 as in getstr.
 */
typedef bool (*parser_strdict_entry_fn)(const char *key, const char *value, void *cb_data);
typedef bool (*parser_strdict_foreach_fn)(parser_context *context, const char *key1, const char *key2, parser_strdict_entry_fn lambda, void *cb_data);

/*
 * Generalized mapping across dictionaries.  Arguably strdict_foreach() is a
 * special case of keymap().  key1, key2 as in getstr.
 */
typedef bool (*parser_keymap_key_fn)(const char *key, void *cb_data);
typedef bool (*parser_keymap_fn)(parser_context *context, const char *key1, const char *key2, parser_keymap_key_fn lambda, void *cb_data);

typedef struct {
    const char *name;
    parser_parse_file_fn parse_file;
    parser_fini_fn fini;

    parser_have_section_fn havesection;
    parser_getstr_fn getstr;
    parser_strarray_foreach_fn strarray_foreach;
    parser_strdict_foreach_fn strdict_foreach;
    parser_keymap_fn keymap;
} parser_plugin;

extern parser_plugin dson_parser;
extern parser_plugin json_parser;
extern parser_plugin yaml_parser;

static inline bool parse_agnostic(const char *filename, parser_plugin **plugin_out, parser_context **context_out)
{
    size_t l = strlen(filename) - 1;

    *plugin_out = NULL;
    *context_out = NULL;

    if (filename[l--] == 'n' && filename[l--] == 'o' && filename[l--] == 's') {
        if (filename[l] == 'd') {
            *plugin_out = &dson_parser;
        } else if (filename[l] == 'j') {
            *plugin_out = &json_parser;
        }
    }

    if (*plugin_out == NULL) {
        *plugin_out = &yaml_parser;
    }

    return (*plugin_out)->parse_file(context_out, filename);
}

#endif /* _LIBRPMINSPECT_PARSER_H */

#ifdef __cplusplus
}
#endif
