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

/*
 * This header defines types used by librpminspect
 */

#include <regex.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <rpm/rpmlib.h>

#ifndef _LIBRPMINSPECT_TYPES_H
#define _LIBRPMINSPECT_TYPES_H

/*
 * List of strings. Used by some of the inspections.
 */
typedef struct _string_entry_t {
    char *data;
    TAILQ_ENTRY(_string_entry_t) items;
} string_entry_t;

typedef TAILQ_HEAD(string_entry_s, _string_entry_t) string_list_t;

/*
 * A file is information about a file in an RPM payload.
 *
 * If fullpath is not NULL, it is the absolute path of the unpacked file.
 * Not every file is unpacked (e.g., block and char special files are skipped).
 * The ownership and permissions of the unpacked file may not match the
 * intended owner and mode from the RPM metadata.
 *
 * "st" is the metadata about the file, as described by the RPM payload. st not
 * necessarily match the description of the file in the RPM header.
 *
 * The rpm_header field is shared by multiple files. Each file entry must
 * call headerFree to dereference the header.
 *
 * idx is the index for this file into the RPM array tags such as RPMTAG_FILESIZES.
 */
typedef struct _rpmfile_entry_t {
    Header rpm_header;
    char *fullpath;
    struct stat st;
    int idx;
    TAILQ_ENTRY(_rpmfile_entry_t) items;
} rpmfile_entry_t;

typedef TAILQ_HEAD(rpmfile_s, _rpmfile_entry_t) rpmfile_t;

/*
 * A peer is a mapping of a built RPM from the before and after builds.
 * We can expand this struct as necessary based on what tests need to
 * reference.
 */
typedef struct _rpmpeer_entry_t {
    Header before_hdr;        /* RPM header of the before package */
    Header after_hdr;         /* RPM header of the after package */
    char *before_rpm;         /* full path to the before RPM */
    char *after_rpm;          /* full path to the after RPM */
    rpmfile_t *before_files;  /* list of files in the payload of the before RPM */
    rpmfile_t *after_files;   /* list of files in the payload of the after RPM */
    TAILQ_ENTRY(_rpmpeer_entry_t) items;
} rpmpeer_entry_t;

typedef TAILQ_HEAD(rpmpeer_s, _rpmpeer_entry_t) rpmpeer_t;

/*
 * And individual inspection result and the list to hold them.
 */
typedef enum _severity_t {
    RESULT_OK     = 0,
    RESULT_INFO   = 1,
    RESULT_WAIVED = 2,
    RESULT_VERIFY = 3,
    RESULT_BAD    = 4
} severity_t;

typedef enum _waiverauth_t {
    NOT_WAIVABLE         = 0,
    WAIVABLE_BY_ANYONE   = 1,
    WAIVABLE_BY_SECURITY = 2,
    WAIVABLE_BY_RELENG   = 3
} waiverauth_t;

typedef struct _results_entry_t {
    severity_t severity;      /* see results.h */
    waiverauth_t waiverauth;  /* who can waive an inspection result */
    char *header;             /* header string for reporting */
    char *msg;                /* the result message */
    char *screendump;         /* screendump (optional, can be NULL) */
    char *remedy;             /* suggested correction for the result */
    TAILQ_ENTRY(_results_entry_t) items;
} results_entry_t;

typedef TAILQ_HEAD(results_s, _results_entry_t) results_t;

/*
 * Configuration and state instance for librpminspect run.
 * Applications using librpminspect should initialize the
 * library and retain this structure through the run of
 * their program.  You need to free this struct on exit.
 */
struct rpminspect {
    char *cfgfile;             /* full path to configuration file */
    char *workdir;             /* full path to working directory */
    char *worksubdir;          /* within workdir, where these builds go */

    /* Runtime data used by tests */
    char *licensedb;           /* full path to the license database */

    /* Koji information (from config file) */
    char *kojihub;             /* URL of Koji hub */
    char *kojidownload;        /* URL to access Koji build artifacts */

    /* Information used by different tests */
    string_list_t *badwords;   /* Space-delimited list of words prohibited
                                * from certain package strings.
                                */
    char *vendor;              /* Required vendor string */
    char *buildhost_subdomain; /* Required subdomain for buildhosts */

    regex_t *elf_path_include;
    regex_t *elf_path_exclude;
    regex_t *manpage_path_include;
    regex_t *manpage_path_exclude;
    regex_t *xml_path_include;
    regex_t *xml_path_exclude;

    /* Options specified by the user */
    char *before;              /* before build ID arg given on cmdline */
    char *after;               /* after build ID arg given on cmdline */
    uint64_t tests;            /* which tests to run (default: ALL) */
    bool verbose;              /* verbose inspection output? */

    /* accumulated data of the build set */
    Header before_srpm_hdr;    /* RPM header of the before src package */
    Header after_srpm_hdr;     /* RPM header of the after src package */
    char *before_srpm;         /* full path to the before source RPM file */
    char *after_srpm;          /* full path to the after source RPM file */
    rpmpeer_t *peers;          /* list of binary packages */

    /* inspection results */
    results_t *results;
};

/*
 * Definition for an output format.
 */
struct format {
    /* The output format type from output.h */
    int type;

    /* short name of the format */
    char *name;

    /* output driver function */
    void (*driver)(const results_t *, const char *);

    /* OPTIONAL: long description of the format (displayed in --help) */
    char *desc;
};

/*
 * Definition for an inspection.  Inspections are assigned a flag (see
 * inspect.h), a short name, and a function pointer to the driver.  The
 * driver function needs to take a struct rpminspect pointer as the only
 * argument.  The driver returns true on success and false on failure.
 */
struct inspect {
    /* the inspection flag from inspect.h */
    uint64_t flag;

    /* short name of inspection */
    char *name;

    /*
     * Does this inspection require a before and after package?
     * Some inspections run just against a single build, which
     * we can use when the user wants to run rpminspect against
     * a single build.
     *
     * True if this inspection is for a single build (which is
     * always the 'after' build throughout the code).
     */
    bool single_build;

    /* the driver function for the inspection */
    bool (*driver)(struct rpminspect *);

    /* OPTIONAL: long description of the inspection (displayed in --help) */
    char *desc;
};

/*
 * List of RPMs from a Koji build.  Only the information we need.
 */
typedef struct _koji_rpmlist_entry_t {
    char *arch;
    char *name;
    char *version;
    char *release;
    TAILQ_ENTRY(_koji_rpmlist_entry_t) items;
} koji_rpmlist_entry_t;

typedef TAILQ_HEAD(koji_rpmlist_s, _koji_rpmlist_entry_t) koji_rpmlist_t;

/*
 * Koji build structure.  This is determined by looking at the
 * output of a getBuild XMLRPC call to a Koji hub.
 *
 * You can examine example values by running xmlrpc on the command
 * line and giving it a build specification, e.g.:
 *     xmlrpc KOJI_HUB_URL getBuild NVR
 * Not all things returned are represented in this struct.
 */
struct koji_build {
    /* These are all relevant to the name of the package */
    char *package_name;
    char *epoch;
    char *name;
    char *version;
    char *release;
    char *nvr;

    /* The source used to drive this build (usually a VCS link) */
    char *source;

    /* Koji-specific information about the build */
    char *creation_time;
    char *completion_time;
    int package_id;
    int id;
    int build_id;
    int state;
    double completion_ts;
    int owner_id;
    char *owner_name;
    char *start_time;
    int creation_event_id;
    double start_ts;
    int volume_id;
    double creation_ts;
    int task_id;

    /* Where to find the resulting build artifacts */
    char *volume_name;

    /* List of RPMs in this build */
    koji_rpmlist_t *rpms;
};

#endif
