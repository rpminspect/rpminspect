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

/*
 * This header defines types used by librpminspect
 */

#include <regex.h>
#include <stdint.h>
#include <stdbool.h>
#include <search.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/capability.h>
#include <rpm/rpmlib.h>
#include <libkmod.h>

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
 * "localpath" is file path from the RPM payload, and "st" is the metadata about the file,
 * as described by the RPM payload. localpath and st may not necessarily match
 * the description of the file in the RPM header.
 *
 * The rpm_header field is shared by multiple files. Each file entry must
 * call headerFree to dereference the header.
 *
 * idx is the index for this file into the RPM array tags such as RPMTAG_FILESIZES.
 *
 * type is the MIME type string that you would get from 'file --mime-type'.
 *
 * cap is the getcap() value for the file.
 *
 * checksum is a string containing the human-readable checksum digest
 *
 * probably_moved_path is true if the file moved path locations between the before
 * after after build, false otherwise
 */
typedef struct _rpmfile_entry_t {
    Header rpm_header;
    char *fullpath;
    char *localpath;
    struct stat st;
    int idx;
    char *type;
    char *checksum;
    cap_t cap;
    struct _rpmfile_entry_t *peer_file;
    bool probably_moved_path;
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
    char *before_root;        /* full path to the before RPM extracted root dir */
    char *after_root;         /* full path to the after RPM extracted root dir */
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
    WAIVABLE_BY_SECURITY = 2
} waiverauth_t;

typedef enum _verb_t {
    VERB_NIL = 0,       /* not used, same as "not set" */
    VERB_ADDED = 1,     /* new file or metadata */
    VERB_REMOVED = 2,   /* removed file or metadata */
    VERB_CHANGED = 3,   /* changed file or metadata */
    VERB_FAILED = 4     /* check failing */
} verb_t;

/*
 * struct to make it easier to make multiple calls to add_result()
 * See the results_entry_t for details.
 */
struct result_params {
    severity_t severity;
    waiverauth_t waiverauth;
    const char *header;
    char *msg;
    char *details;
    const char *remedy;
    verb_t verb;
    const char *noun;
    const char *arch;
    const char *file;
};

typedef struct _results_entry_t {
    severity_t severity;      /* see results.h */
    waiverauth_t waiverauth;  /* who can waive an inspection result */
    const char *header;       /* header string for reporting */
    char *msg;                /* the result message */
    char *details;            /* details (optional, can be NULL) */
    const char *remedy;       /* suggested correction for the result */
    verb_t verb;              /* verb indicating what happened */
    char *noun;               /* noun impacted by 'verb', one line
                                 (e.g., a file path or an RPM dependency
                                        string) */
    char *arch;               /* architecture impacted (${ARCH}) */
    char *file;               /* file impacted (${FILE}) */
    TAILQ_ENTRY(_results_entry_t) items;
} results_entry_t;

typedef TAILQ_HEAD(results_s, _results_entry_t) results_t;

/*
 * Known types of Koji builds
 */
typedef enum _koji_build_type_t {
    KOJI_BUILD_NULL = 0,       /* initializer, not an actual build */
    KOJI_BUILD_IMAGE = 1,      /* not supported */
    KOJI_BUILD_MAVEN = 2,      /* not supported */
    KOJI_BUILD_MODULE = 3,
    KOJI_BUILD_RPM = 4,
    KOJI_BUILD_WIN = 5         /* not supported */
} koji_build_type_t;

/*
 * stat-whitelist for a product release. Used by some of the inspections.
 */
typedef enum _stat_whitelist_field_t {
    MODE = 0,
    OWNER = 1,
    GROUP = 2,
    FILENAME = 3
} stat_whitelist_field_t;

typedef struct _stat_whitelist_entry_t {
    mode_t mode;
    char *owner;
    char *group;
    char *filename;
    TAILQ_ENTRY(_stat_whitelist_entry_t) items;
} stat_whitelist_entry_t;

typedef TAILQ_HEAD(stat_whitelist_entry_s, _stat_whitelist_entry_t) stat_whitelist_t;

/*
 * caps-whitelist for a product release.  Used by some of the inspections.
 */
typedef enum _caps_whitelist_field_t {
    PACKAGE = 0,
    FILEPATH = 1,
    EQUAL = 2,
    CAPABILITIES = 3
} caps_whitelist_field_t;

typedef struct _caps_filelist_entry_t {
    char *path;
    char *caps;
    TAILQ_ENTRY(_caps_filelist_entry_t) items;
} caps_filelist_entry_t;

typedef TAILQ_HEAD(caps_filelist_entry_s, _caps_filelist_entry_t) caps_filelist_t;

typedef struct _caps_whitelist_entry_t {
    char *pkg;
    caps_filelist_t *files;
    TAILQ_ENTRY(_caps_whitelist_entry_t) items;
} caps_whitelist_entry_t;

typedef TAILQ_HEAD(caps_whitelist_entry_s, _caps_whitelist_entry_t) caps_whitelist_t;

/* Spec filename matching types */
typedef enum _specname_match_t {
    MATCH_NULL = 0,
    MATCH_FULL = 1,
    MATCH_PREFIX = 2,
    MATCH_SUFFIX = 3
} specname_match_t;

typedef enum _specname_primary_t {
    PRIMARY_NULL = 0,
    PRIMARY_NAME = 1,
    PRIMARY_FILENAME = 2
} specname_primary_t;

/* RPM header cache so we don't balloon out our memory */
typedef struct _header_cache_entry_t {
    char *pkg;
    Header hdr;
    TAILQ_ENTRY(_header_cache_entry_t) items;
} header_cache_entry_t;

/* Product release string favoring */
typedef enum _favor_release_t {
    FAVOR_NONE = 0,
    FAVOR_OLDEST = 1,
    FAVOR_NEWEST = 2
} favor_release_t;

typedef TAILQ_HEAD(header_cache_entry_s, _header_cache_entry_t) header_cache_t;

/*
 * Configuration and state instance for librpminspect run.
 * Applications using librpminspect should initialize the
 * library and retain this structure through the run of
 * their program.  You need to free this struct on exit.
 */
struct rpminspect {
    char *cfgfile;             /* full path to configuration file */
    char *workdir;             /* full path to working directory */
    char *profiledir;          /* full path to profiles directory */
    char *worksubdir;          /* within workdir, where these builds go */

    /* Vendor data */
    char *vendor_data_dir;     /* main vendor data directory */
    char *licensedb;           /* name of file under licenses/ to use */
    favor_release_t favor_release;

    /* Populated at runtime for the product release */
    stat_whitelist_t *stat_whitelist;
    caps_whitelist_t *caps_whitelist;

    /* Koji information (from config file) */
    char *kojihub;             /* URL of Koji hub */
    char *kojiursine;          /* URL to access packages built in Koji */
    char *kojimbs;             /* URL to access module packages in Koji */

    /* Information used by different tests */
    string_list_t *badwords;   /* Space-delimited list of words prohibited
                                * from certain package strings.
                                */
    char *vendor;              /* Required vendor string */

    /* Required subdomain for buildhosts -- multiple subdomains allowed */
    string_list_t *buildhost_subdomain;

    /*
     * Optional: if not NULL, contains list of path prefixes for files
     * that are of security concern.
     */
    string_list_t *security_path_prefix;

    /*
     * Optional: if not NULL, contains a list of filename extensions
     * for C and C++ header files.
     */
    string_list_t *header_file_extensions;

    /*
     * Optional: Lists of path substrings and directories to forbid.
     */
    string_list_t *forbidden_path_prefixes;
    string_list_t *forbidden_path_suffixes;
    string_list_t *forbidden_directories;

    /*
     * Optional: if not NULL, contains a list of functions known to have
     * IPv6-compatibility issues.
     */
    string_list_t *ipv6_blacklist;

    /* Optional: if not NULL, contains list of architectures */
    string_list_t *arches;

    regex_t *elf_path_include;
    regex_t *elf_path_exclude;
    regex_t *manpage_path_include;
    regex_t *manpage_path_exclude;
    regex_t *xml_path_include;
    regex_t *xml_path_exclude;

    /* Where desktop entry files live */
    char *desktop_entry_files_dir;

    /* Executable path prefixes and required ownership */
    string_list_t *bin_paths;
    char *bin_owner;
    char *bin_group;

    /* Optional: Forbidden file owners and groups */
    string_list_t *forbidden_owners;
    string_list_t *forbidden_groups;

    /* List of shells to check script syntax */
    string_list_t *shells;

    /* Optional: file size change threshold for inc/dec reporting (%) */
    char *size_threshold;

    /* Optional: ELF LTO symbol prefixes */
    string_list_t *lto_symbol_name_prefixes;

    /* Spec filename matching type */
    specname_match_t specmatch;
    specname_primary_t specprimary;

    /* hash table of product release -> JVM major versions */
    struct hsearch_data *jvm;
    string_list_t *jvm_keys;

    /* hash table of annocheck tests */
    struct hsearch_data *annocheck;
    string_list_t *annocheck_keys;

    /* hash table of path migrations */
    struct hsearch_data *pathmigration;
    string_list_t *pathmigration_keys;

    /* hash table of product release regexps */
    struct hsearch_data *products;
    string_list_t *product_keys;

    /* list of paths to ignore (these strings allow glob(3) syntax) */
    string_list_t *ignores;

    /* Options specified by the user */
    char *before;              /* before build ID arg given on cmdline */
    char *after;               /* after build ID arg given on cmdline */
    uint64_t tests;            /* which tests to run (default: ALL) */
    bool verbose;              /* verbose inspection output? */

    /* Failure threshold */
    severity_t threshold;
    severity_t worst_result;

    /* The product release we are inspecting against */
    char *product_release;

    /* The type of Koji build we are looking at */
    /*
     * NOTE: rpminspect works with RPMs at the lowest level, so
     * build types that are other collections of RPMs may be
     * supported. But supporting non-RPM containers in rpminspect
     * is not really in scope.
     */
    koji_build_type_t buildtype;

    /* accumulated data of the build set */
    rpmpeer_t *peers;               /* list of packages */
    header_cache_t *header_cache;   /* RPM header cache */
    char *before_rel;               /* before Release w/o %{?dist} */
    char *after_rel;                /* after Release w/o ${?dist} */

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
};

/*
 * List of RPMs from a Koji build.  Only the information we need.
 */
typedef struct _koji_rpmlist_entry_t {
    char *arch;
    char *name;
    char *version;
    char *release;
    int epoch;
    long long int size;
    TAILQ_ENTRY(_koji_rpmlist_entry_t) items;
} koji_rpmlist_entry_t;

typedef TAILQ_HEAD(koji_rpmlist_s, _koji_rpmlist_entry_t) koji_rpmlist_t;

/*
 * List of build IDs from a Koji build.
 */
typedef struct _koji_buildlist_entry_t {
    int build_id;
    char *package_name;
    char *owner_name;
    int task_id;
    int state;
    char *nvr;
    char *start_time;
    int creation_event_id;
    char *creation_time;
    char *epoch;
    int tag_id;
    char *completion_time;
    char *tag_name;
    char *version;
    int volume_id;
    char *release;
    int package_id;
    int owner_id;
    int id;
    char *volume_name;
    char *name;

    /* List of RPMs in this build */
    koji_rpmlist_t *rpms;

    TAILQ_ENTRY(_koji_buildlist_entry_t) builditems;
} koji_buildlist_entry_t;

typedef TAILQ_HEAD(koji_buildlist_s, _koji_buildlist_entry_t) koji_buildlist_t;

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
    /* These are all relevant to the name of the build */
    char *package_name;
    int epoch;
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
    int state;
    double completion_ts;
    int owner_id;
    char *owner_name;
    char *start_time;
    int creation_event_id;
    double start_ts;
    double creation_ts;
    int task_id;

    /* Where to find the resulting build artifacts */
    int volume_id;
    char *volume_name;

    /*
     * Original source URL, for some reason
     * (not present for module builds)
     */
    char *original_url;

    /* Content Generator information (currently not used in rpminspect) */
    int cg_id;
    char *cg_name;

    /* Module metadata -- only if this build is a module */
    char *modulemd_str;
    char *module_name;
    char *module_stream;
    int module_build_service_id;
    char *module_version;
    char *module_context;
    char *module_content_koji_tag;

    /* List of build IDs associated with this build */
    koji_buildlist_t *builds;
};

/*
 * Koji task structure.  This is determined by looking at the
 * output of a getTaskInfo XMLRPC call to a Koji hub.
 *
 * You can examine example values by running xmlrpc on the command
 * line and giving it a task ID, e.g.:
 *     xmlrpc KOJI_HUB_URL getTaskInfo ID
 * Not all things returned are represented in this struct.
 */
typedef TAILQ_HEAD(koji_task_list_s, _koji_task_entry_t) koji_task_list_t;

struct koji_task {
    /* members returned from getTaskInfo */
    double weight;
    int parent;
    char *completion_time;
    char *start_time;
    double start_ts;
    bool waiting;
    bool awaited;
    char *label;
    int priority;
    int channel_id;
    int state;
    char *create_time;
    double create_ts;
    int owner;
    int host_id;
    char *method;
    double completion_ts;
    char *arch;
    int id;

    /* Descendent tasks (where files are) */
    koji_task_list_t *descendents;
};

/* A generic list of koji tasks */
typedef struct _koji_task_entry_t {
    /* main task information */
    struct koji_task *task;

    /* results from getTaskResult */
    int brootid;
    string_list_t *srpms;
    string_list_t *rpms;
    string_list_t *logs;

    TAILQ_ENTRY(_koji_task_entry_t) items;
} koji_task_entry_t;

/* Types of files -- used by internal nftw() callbacks */
typedef enum _filetype_t {
    FILETYPE_NULL = 0,
    FILETYPE_EXECUTABLE = 1,
    FILETYPE_ICON = 2
} filetype_t;

/* Kernel module handling */
typedef void (*modinfo_to_entries)(string_list_t *, const struct kmod_list *);
typedef void (*module_alias_callback)(const char *, const string_list_t *, const string_list_t *, void *);

/* mapping of an alias string to a module name */
struct alias_entry_t {
    char *alias;
    char *module;
    TAILQ_ENTRY(alias_entry_t) items;
};

TAILQ_HEAD(alias_list_t, alias_entry_t);

typedef struct _kernel_alias_data {
    size_t num_aliases;
    struct alias_list_t *alias_list;
    struct hsearch_data *alias_table;
} kernel_alias_data_t;

/* Types of workdirs */
typedef enum _workdir_t {
    NULL_WORKDIR = 0,          /* unused                    */
    LOCAL_WORKDIR = 1,         /* locally cached koji build */
    TASK_WORKDIR = 2,          /* like for scratch builds   */
    BUILD_WORKDIR = 3          /* remote koji build spec    */
} workdir_t;

/**
 * @brief Callback function to pass to foreach_peer_file.
 *
 * Given the program's main struct rpminspect and a single
 * rpmfile_entry_t, perform a defined action and return true if the
 * action was successful and false otherwise.  This is used for
 * callback functions in inspection functions.  You can iterate over
 * each file in a package and perform the inspection.  True means
 * everything passed, false means something failed.  Since the
 * callback received the struct rpminspect, you can add results as the
 * program returns and not worry about losing inspection details.
 */
typedef bool (*foreach_peer_file_func)(struct rpminspect *, rpmfile_entry_t *);

#endif
