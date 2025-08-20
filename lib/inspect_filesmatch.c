/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <assert.h>
#include <rpm/rpmmacro.h>

#include "rpminspect.h"

/* Globals */
static bool result = true;
static string_hash_t *files_globs = NULL;
static string_hash_t *files_dirs = NULL;
static string_hash_t *files_excludes = NULL;

enum {
    FILES_GLOBS = 0,
    FILES_DIRS = 1,
    FILES_EXCLUDES = 2
};

/* Return string describing %files entry type */
static char *files_entry_description(const int type)
{
    if (type == FILES_GLOBS) {
        return "   glob";
    } else if (type == FILES_DIRS) {
        return "    dir";
    } else if (type == FILES_EXCLUDES) {
        return "exclude";
    } else {
        return "unknown";
    }
}

/* Helper function to save strings in a specified hash table */
static void save_pathspec(const char *s, const int type)
{
    string_hash_t *hash = NULL;
    string_hash_t *entry = NULL;
    char *expanded = NULL;

    if (s == NULL) {
        return;
    }

    /* sometimes we end up with empty strings, ignore */
    if (strlen(s) == 0) {
        return;
    }

    /* alright, we need at least some path separator in a pathspec */
    if (!strchr(s, PATH_SEP)) {
        return;
    }

    /* determine which hash table to use */
    if (type == FILES_GLOBS) {
        hash = files_globs;
    } else if (type == FILES_DIRS) {
        hash = files_dirs;
    } else if (type == FILES_EXCLUDES) {
        hash = files_excludes;
    }

    /* expand any macros in the string */
    expanded = calloc(1, BUFSIZ);
    assert(expanded != NULL);

    if (rpmExpandMacros(NULL, s, &expanded, 0) < 0) {
        warn("rpmExpandMacros");
    }

    /* look for the string first */
    HASH_FIND_STR(hash, s, entry);

    /* if we don't have the string yet, add it to the hash table */
    if (entry == NULL) {
        entry = xalloc(sizeof(*entry));
        assert(entry != NULL);
        entry->data = strdup(expanded);
        DEBUG_PRINT("type:pathspec: %s:|%s|\n", files_entry_description(type), entry->data);
        HASH_ADD_KEYPTR(hh, hash, entry->data, strlen(entry->data), entry);
    }

    /* cleanup */
    free(expanded);

    return;
}

/*
 * Trim any leading modifiers on %files entry lines that are not
 * delimited by spaces from the actual pathspec.  For instance, a line
 * like this:
 *
 *     %config(noreplace)/etc/passwd
 *
 * Would have the %config() part trimmed.  In cases where there is a
 * space between "%config(noreplace)" and "/etc/passwd, the main block
 * below would handle skipping the modifier.
 */
static char *trim_files_modifiers(char *s)
{
    char *r = s;

    if (r == NULL) {
        return NULL;
    }

    if (strprefix(r, SPEC_FILES_ATTR) ||
        strprefix(r, SPEC_FILES_CONFIG) ||
        strprefix(r, SPEC_FILES_VERIFY) ||
        strprefix(r, SPEC_FILES_LANG) ||
        strprefix(r, SPEC_FILES_CAPS)) {
        if (strchr(r, '(') && strchr(r, ')')) {
            r = strchr(r, ')');
        }

        while (*r != PATH_SEP && *r != '\0') {
            *r = ' ';
            r++;
        }
    }

    return r;
}

/*
 * Process %doc and %license lines in %files blocks.
 */
static void process_doc_lines(const char *name, const string_list_t *tokens, const char *macro, const char *path)
{
    char *prefix = NULL;
    char *p = NULL;
    string_entry_t *token = NULL;

    assert(name != NULL);
    assert(tokens != NULL);
    assert(macro != NULL);
    assert(path != NULL);

    /* directory prefix */
    xasprintf(&prefix, "%%{%s}/%s", macro, name);
    assert(prefix != NULL);

    /* expand each file listed on the macro line */
    TAILQ_FOREACH(token, tokens, items) {
        if (!strcmp(token->data, macro)) {
            continue;
        } else if (strprefix(token->data, "%")) {
            warn("*** unexpanded macro in %s line: %s", macro, token->data);
            continue;
        }

        /* build the full path */
        p = joindelim(PATH_SEP, prefix, token->data, NULL);
        assert(p != NULL);

        /* save this glob in the right hash table */
        save_pathspec(p, FILES_GLOBS);

        free(p);
    }

    free(prefix);
    return;
}

/*
 * Read all of the %files section in spec files and gather the
 * macro-expanded entries.
 */
static void gather_files_entries(struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    string_list_t *speclines = NULL;
    string_entry_t *line = NULL;
    string_list_t *tokens = NULL;
    string_entry_t *token = NULL;
    bool found = false;
    bool verify = false;
    const char *name = NULL;
    char *pathspec = NULL;
    int type = FILES_GLOBS;

    assert(ri != NULL);

    /* Build the file glob list first */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (!headerIsSource(peer->after_hdr)) {
            continue;
        }

        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        name = headerGetString(peer->after_hdr, RPMTAG_NAME);
        DEBUG_PRINT("package: %s-%s-%s\n", name, headerGetString(peer->after_hdr, RPMTAG_VERSION), headerGetString(peer->after_hdr, RPMTAG_RELEASE));

        /*
         * Iterate over the file list of the SRPM and read the spec file.
         * There should only be one spec file, but if there is a future
         * with multiple spec files in a single SRPM then I sure hope to
         * be a USCG licensed captain by then.
         */
        TAILQ_FOREACH(file, peer->after_files, items) {
            /* find the spec file and read the lines */
            if (strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
                speclines = read_spec(ri, file->fullpath);

                if (speclines == NULL || TAILQ_EMPTY(speclines)) {
                    warn("*** read_spec");
                    continue;
                }

                break;
            }
        }

        if (speclines == NULL || TAILQ_EMPTY(speclines)) {
            continue;
        }

        /* read in all of the glob(7) lines from %files sections */
        TAILQ_FOREACH(line, speclines, items) {
            type = FILES_GLOBS;

            /* skip empty lines */
            if (!strcmp(line->data, "")) {
                continue;
            }

            /* when found is true, we are reading %files entries */
            if (found) {
                /* we have a glob(7) specification */
                pathspec = line->data;

                /* look for %files sections and skip other sections */
                if (strprefix(pathspec, SPEC_SECTION_FILES)) {
                    /* a subpackage %files section, keep reading */
                    continue;
                } else if (strprefix(pathspec, SPEC_SECTION_DESCRIPTION) ||
                           strprefix(pathspec, SPEC_SECTION_PACKAGE) ||
                           strprefix(pathspec, SPEC_SECTION_PREP) ||
                           strprefix(pathspec, SPEC_SECTION_BUILD) ||
                           strprefix(pathspec, SPEC_SECTION_INSTALL) ||
                           strprefix(pathspec, SPEC_SECTION_CHECK) ||
                           strprefix(pathspec, SPEC_SECTION_PRE) ||
                           strprefix(pathspec, SPEC_SECTION_PREUN) ||
                           strprefix(pathspec, SPEC_SECTION_POST) ||
                           strprefix(pathspec, SPEC_SECTION_POSTUN) ||
                           strprefix(pathspec, SPEC_SECTION_TRIGGERUN) ||
                           strprefix(pathspec, SPEC_SECTION_CHANGELOG)) {
                    /* we reached the end of the %files section(s) */
                    found = false;
                    break;
                }

                /* split up the line for further processing */
                tokens = strsplit(pathspec, " \t");

                if (tokens == NULL || TAILQ_EMPTY(tokens)) {
                    continue;
                }

                /* walk over each token in the %files line */
                type = FILES_GLOBS;
                verify = false;

                TAILQ_FOREACH(token, tokens, items) {
                    if (!strcmp(token->data, SPEC_FILES_DOC)) {
                        process_doc_lines(name, tokens, SPEC_FILES_DOC, SPEC_FILES_DOCDIR);
                        break;
                    } else if (!strcmp(token->data, SPEC_FILES_LICENSE)) {
                        process_doc_lines(name, tokens, SPEC_FILES_LICENSE, SPEC_FILES_LICENSEDIR);
                        break;
                    } else if (verify) {
                        /* skip the tokens inside the %verify () expression */
                        if (strsuffix(token->data, ")")) {
                            verify = false;
                        }

                        continue;
                    } else if (strprefix(token->data, SPEC_FILES_ATTR) ||
                               strprefix(token->data, SPEC_FILES_CONFIG) ||
                               strprefix(token->data, SPEC_FILES_LANG) ||
                               strprefix(token->data, SPEC_FILES_CAPS)) {
                        /* skip %attr, %config, %lang, and %caps */
                        continue;
                    } else if (!verify && strprefix(token->data, SPEC_FILES_VERIFY)) {
                        /* skip %verify but note that it's a special case */
                        verify = true;
                        continue;
                    } else if (strprefix(token->data, SPEC_FILES_DIR)) {
                        /*
                         * this is a %dir specification so we
                         * record it in a different hash table
                         * because checking actual files in the
                         * binary RPMs will be checking path
                         * prefixes against these vs glob matching
                         */
                        type = FILES_DIRS;
                    } else if (strprefix(token->data, SPEC_FILES_EXCLUDE)) {
                        /*
                         * this is an %exclude specification which
                         * is a known glob that *SHOULD NOT* be
                         * present in built RPMs
                         */
                        type = FILES_EXCLUDES;
                    } else {
                        /* at this point we should have the pathspec portion */
                        pathspec = token->data;
                    }
                }

                /*
                 * there is a possibility of %files modifiers
                 * existing without using space delimiters, so
                 * trim those
                 */
                pathspec = trim_files_modifiers(pathspec);

                /* trim and leading or trailing whitespace */
                pathspec = strtrim(pathspec);

                if (pathspec == NULL) {
                    /* no reason to continue if there's nothing here */
                    continue;
                }

                save_pathspec(pathspec, type);

                /* cleanup */
                list_free(tokens, free);
            } else if (!found && strprefix(line->data, SPEC_SECTION_FILES)) {
                /* we found %files, start reading lines */
                found = true;
            }
        }

        list_free(speclines, free);
    }

    return;
}

/*
 * Main driver for the 'filesmatch' inspection.
 */
bool inspect_filesmatch(struct rpminspect *ri)
{
    struct result_params params;

    assert(ri != NULL);

    /* Read in all of the %files entries */
    gather_files_entries(ri);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_FILESMATCH;
    params.verb = VERB_OK;

    /* Report a single OK message if everything was fine */
    if (result) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    }

    /* cleanup */
    free_string_hash(files_dirs);
    free_string_hash(files_globs);
    free_string_hash(files_excludes);

    return result;
}
