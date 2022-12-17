/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/* This file is derived from libtyranny:
 * https://github.com/frozencemetery/libtyranny/ */

#ifndef _LIBRPMINSPECT_TYRANNY_H
#define _LIBRPMINSPECT_TYRANNY_H

#include <yaml.h>

/* This is a private header, so enums are cool. */
typedef enum {
    Y_UNINITIALIZED = 0,
    Y_STRING,
    Y_ARRAY, /* "sequence", in YAML speak */
    Y_DICT, /* "mapping", in YAML parlance */
} y_type;

/* This is a public type.  Feel free to traverse it yourself, if you want. */
typedef struct y_value {
    y_type type;
    union {
        char *string; /* \0-terminated. */
        struct y_value **array; /* NULL-terminated. */

        /* keys is NULL-terminated.  values might contain NULLs because YAML
         * allows you to just not map a dict entry to anything. */
        struct {
            char **keys;
            struct y_value **values;
        } dict;
    } v; /* Anonymous unions would require C11 :( */
} y_value;

/* Internal context structure for lookahead. */
typedef struct {
    yaml_parser_t *parser;
    bool have_lookahead;
    yaml_token_t token;
} context;

#endif /* _LIBRPMINSPECT_TYRANNY_H */
