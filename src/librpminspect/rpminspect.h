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
 * This header includes the API definition for librpminspect.
 */

#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <iniparser.h>
#include <rpm/header.h>
#include "constants.h"
#include "types.h"
#include "inspect.h"
#include "results.h"
#include "output.h"
#include "readelf.h"

#ifndef _LIBRPMINSPECT_RPMINSPECT_H
#define _LIBRPMINSPECT_RPMINSPECT_H

/* List of all inspections (inspect.c) */
extern struct inspect inspections[];

/* List of all output format types (output.c) */
extern struct format formats[];

/* Macros */
#define xasprintf(dest, ...) {                   \
    int _xasprintf_result;                       \
    *(dest) = NULL;                              \
    _xasprintf_result = asprintf((dest), __VA_ARGS__);\
    assert(_xasprintf_result != -1);             \
}

/*
 * Build identifier strings (used in paths)
 * The enum values map to the build_desc array index.
 */
enum { BEFORE_BUILD, AFTER_BUILD };

/* Common functions */

/* init.c */
int init_rpminspect(struct rpminspect *, const char *);

/* free.c */
void free_rpminspect(struct rpminspect *);

/* listfuncs.c */
string_list_t * list_difference(const string_list_t *, const string_list_t *);
string_list_t * list_intersection(const string_list_t *, const string_list_t *);
string_list_t * list_union(const string_list_t *, const string_list_t *);
string_list_t * list_symmetric_difference(const string_list_t *, const string_list_t *);
typedef void (*list_entry_data_free_func)(void *);
void list_free(string_list_t *, list_entry_data_free_func);
size_t list_len(const string_list_t *);
string_list_t * list_sort(const string_list_t *);
string_list_t * list_copy(const string_list_t *);

/* local.c */
bool is_local_build(const char *);

/* koji.c */
koji_rpmlist_t *init_koji_rpmlist(void);
void free_koji_rpmlist(koji_rpmlist_t *);
void init_koji_build(struct koji_build *);
void free_koji_build(struct koji_build *);
struct koji_build *get_koji_build(struct rpminspect *, const char *);

/* mkdirp.c */
int mkdirp(char *, mode_t);

/* rmtree.c */
int rmtree(const char *, const bool, const bool);

/* strfuncs.c */
bool strprefix(const char *, const char *);
bool strsuffix(const char *, const char *);
int printwrap(const char *, const size_t, const unsigned int, FILE *);
bool versioned_match(const char *, Header, const char *, Header);
char *strseverity(const severity_t);
char *strwaiverauth(const waiverauth_t);
char *strreplace(const char *, const char *, const char *);

/* badwords.c */
bool has_bad_word(const char *, const string_list_t *);

/* copyfile.c */
int copyfile(const char *, const char *, bool, bool);

/* rpm.c */
int init_librpm(void);
int get_rpm_header(const char *, Header *);

/* peers.c */
rpmpeer_t *init_rpmpeer(void);
void free_rpmpeer(rpmpeer_t *);
void add_peer(rpmpeer_t **, int, const char *, Header *);

/* files.c */
void free_files(rpmfile_t *files);
rpmfile_t * extract_rpm(const char *, Header);
const char * get_file_path(const rpmfile_entry_t *file);
bool process_file_path(const rpmfile_entry_t *, regex_t *, regex_t *);
void find_file_peers(rpmfile_t *, rpmfile_t *);

/* tty.c */
size_t tty_width(void);

/* results.c */
results_t *init_results(void);
void free_results(results_t *);
void add_result(results_t **, severity_t, waiverauth_t, char *, char *, char *, char *);

/* output_text.c */
void output_text(const results_t *, const char *);

/* output_json.c */
void output_json(const results_t *, const char *);

#endif
