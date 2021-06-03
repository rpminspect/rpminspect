/*
 * Copyright © 2021 Red Hat, Inc.
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

#include "uthash.h"

#ifndef _LIBRPMINSPECT_SECRULES_H
#define _LIBRPMINSPECT_SECRULES_H

/*
 * Vendor security rule types.  These are used as 'type' values in the
 * security rules structure after reading in configuration data.  The
 * corresponding configuration file string is in a comment for each
 * rule type (e.g., "caps" or "fortifysource").
 */
enum secrule_type {
    /* not used */
    SECRULE_NULL = 0,

    /*
     * caps
     * Any inspection that looks at capabilities(7) values.
     */
    SECRULE_CAPS = 1,

    /*
     * execstack
     * ELF object contains an executable stack or built without
     * GNU_STACK.
     */
    SECRULE_EXECSTACK = 2,

    /*
     * relro
     * ELF object loses partial or full GNU_RELRO protection.
     */
    SECRULE_RELRO = 3,

    /*
     * fortifysource
     * ELF object loses -D_FORTIFY_SOURCE protection.
     */
    SECRULE_FORTIFYSOURCE = 4,

    /*
     * pic
     * ELF objects in static libraries built without -fPIC
     */
    SECRULE_PIC = 5,

    /*
     * textrel
     * ELF object has TEXTREL relocations.
     */
    SECRULE_TEXTREL = 6,

    /*
     * setuid
     * File has CAP_SETUID but is group writable.
     */
    SECRULE_SETUID = 7,

    /*
     * worldwritable
     * File or directory is world writable.
     */
    SECRULE_WORLDWRITABLE = 8,

    /*
     * securitypath
     * File is removed but belonged in a security path prefix as
     * defined in the configuration file.
     */
    SECRULE_SECURITYPATH = 9,

    /*
     * modes
     * File mode does not match expected mode from the fileinfo rules.
     */
    SECRULE_MODES = 10
};

enum secrule_action {
    /* not used */
    SECRULE_ACTION_NULL = 0,

    /* reporting level will be INFO */
    SECRULE_ACTION_INFORM = 1,

    /* reporting level will be VERIFY */
    SECRULE_ACTION_VERIFY = 2,

    /* reporting level will be BAD */
    SECRULE_ACTION_FAIL = 3
};

/*
 * Security rule actions hash table
 * There is one of these for each row in the vendor security
 * definitions file.
 */
typedef struct _secrule_t {
    enum secrule_type type;
    enum secrule_action action;
    UT_hash_handle hh;
} secrule_t;

/*
 * Security hash table
 * This table holds the rows as read from the vendor security
 * definitions files.
 */
typedef struct _security_t {
    char *pkg;
    char *ver;
    char *rel;
    secrule_t *rules;
    UT_hash_handle hh;
} security_t;

#endif
