/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef _LIBRPMINSPECT_SECRULES_H
#define _LIBRPMINSPECT_SECRULES_H

/*
 * Vendor security rule types.  These are used as 'type' values in the
 * security rules structure after reading in configuration data.  The
 * corresponding configuration file string is in a comment for each
 * rule type (e.g., "caps" or "fortifysource").
 */

typedef enum _secrule_type_t {
    /* only used to indicate an unknown rule */
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
    SECRULE_MODES = 10,

    /*
     * virus
     * File contains a virus found by libclamav.
     */
    SECRULE_VIRUS = 11
} secrule_type_t;

#endif
