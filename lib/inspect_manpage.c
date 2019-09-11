/*
 * Copyright (C) 2019  Red Hat, Inc.
 * Author(s):  David Shea <dshea@redhat.com>
 *             David Cantrell <dcantrell@redhat.com>
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

#include <assert.h>
#include <errno.h>
#include <regex.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <rpm/header.h>
#include <rpm/rpmtag.h>

#include <man.h>
#include <mandoc.h>

#include "rpminspect.h"

static regex_t sections_regex;

/* Allocate memory used by inspect_manpage */
bool inspect_manpage_alloc(void)
{
    int reg_result;
    char reg_error[BUFSIZ];
    char *tmp = NULL;

    mchars_alloc();

    /* extract the directory section to match 1, and the filename section to match 2.
     * For the directory section, look for /man<section>
     * For the filename section, look for <name>.<section>.gz
     */
    xasprintf(&tmp, "/man\\([^/]\\+\\)/[^/]\\+\\.\\([^.]\\+\\)\\%s$", GZIPPED_FILENAME_EXTENSION);
    reg_result = regcomp(&sections_regex, tmp, 0);
    free(tmp);
    if (reg_result != 0) {
        regerror(reg_result, &sections_regex, reg_error, sizeof(reg_error));
        fprintf(stderr, "Unable to compile man page path regular expression: %s\n", reg_error);
        inspect_manpage_free();
        return false;
    }

    return true;
}

/* Free the memory used by mandoc */
void inspect_manpage_free(void)
{
    mchars_free();
    regfree(&sections_regex);
}

/*
 * Check that a man page is in the correct directory for its section.
 * The directory section (/usr/share/man/man<section>) must be a prefix
 * of the filename section (manpage.<section>[.gz]). The filename section
 * can include additional trailing characters. e.g., man1/x509.1ssl.gz is valid.
 * man1x/imake.1.gz is not.
 */
bool inspect_manpage_path(const char *path)
{
    /* 0 is the whole match, 1 is the directory section, 2 is the filename section */
    regmatch_t section_matches[3];

    char directory_section[128];
    char filename_section[128];


    /* If there was no match, or if the match is bigger than our buffer,
     * assume something is wrong with the path and return false.
     */
    if (regexec(&sections_regex, path, 3, section_matches, 0) != 0) {
        return false;
    }

    if (section_matches[1].rm_so < 0) {
        return false;
    }

    if (section_matches[2].rm_so < 0) {
        return false;
    }

    if ((unsigned int) (section_matches[1].rm_eo - section_matches[1].rm_so) > sizeof(directory_section)) {
        return false;
    }

    if ((unsigned int) (section_matches[2].rm_eo - section_matches[2].rm_so) > sizeof(directory_section)) {
        return false;
    }

    snprintf(directory_section, sizeof(directory_section), "%.*s",
            section_matches[1].rm_eo - section_matches[1].rm_so,
            path + section_matches[1].rm_so);
    snprintf(filename_section, sizeof(filename_section), "%.*s",
            section_matches[2].rm_eo - section_matches[2].rm_so,
            path + section_matches[2].rm_so);

    return strprefix(filename_section, directory_section);
}

/*
 * Validate a man page file by parsing it with mandoc. Additionally check that
 * the man page is compressed.
 *
 * Returns NULL on success, otherwise returns an error message as a string.
 * The returned string is statically allocated.
 */
char * inspect_manpage_validity(const char *path, const char *localpath)
{
    struct mparse *parser;
    int fd = -1;
    char magic[2];
    struct roff_man *man;
    enum mandoclevel result_tmp;
    enum mandoclevel result = MANDOCLEVEL_OK;

    /* Use a nested function to gather error messages, since the callback doesn't
     * take any parameters that would tie it back to this call's context
     */
    char *error_buffer = NULL;
    size_t error_buffer_size = 0;
    FILE *error_stream;
    void error_handler(enum mandocerr errtype, enum mandoclevel level, const char *file,
            int line, int col, const char *msg)
    {
        fprintf(error_stream, "Error parsing %s:%d:%d: %s: %s: %s\n",
                basename(file), line, col, mparse_strlevel(level), mparse_strerror(errtype), msg);
    }

    /* Initialize the error buffer */
    error_stream = open_memstream(&error_buffer, &error_buffer_size);
    assert(error_stream != NULL);

    /* Allocate a new manpage parsing context */
    parser = mparse_alloc(MPARSE_MAN | MPARSE_UTF8 | MPARSE_LATIN1,
            MANDOCERR_ERROR, error_handler, MANDOC_OS_OTHER, NULL);
    assert(parser != NULL);

    /* Open the file */
    if ((fd = mparse_open(parser, path)) == -1) {
        fprintf(error_stream, "Unable to open man page %s\n", path);
        goto end;
    }

    /* Ensure the file is compressed. The file *should* end in .gz, and if it
     * does make sure that it's actually gzipped.
     */
    if (!strsuffix(path, GZIPPED_FILENAME_EXTENSION)) {
        fprintf(error_stream, "Man page %s does not end in %s\n", path, GZIPPED_FILENAME_EXTENSION);
    } else {
        if (read(fd, magic, sizeof(magic)) != sizeof(magic)) {
            fprintf(error_stream, "Unable to read man page %s\n", path);
            goto end;
        }

        if (magic[0] != '\x1F' || magic[1] != '\x8B') {
            fprintf(error_stream, "man page with %s suffix is not really compressed with gzip\n", GZIPPED_FILENAME_EXTENSION);
        }

        /* Reset the fd and continue */
        lseek(fd, 0, SEEK_SET);
    }

    /* Parse the file */
    result_tmp = mparse_readfd(parser, fd, path);
    if (result_tmp > result) {
        result = result_tmp;
    }

    /* Retrieve the syntax tree */
    mparse_result(parser, &man, NULL);

    /* Validate the man page */
    if (man != NULL) {
        man_validate(man);
    }

    /* Check for validation errors */
    mparse_updaterc(parser, &result);
    if (result > MANDOCLEVEL_OK) {
        fprintf(error_stream, "Errors found validating %s\n", (localpath == NULL) ? path : localpath);
    }

end:
    mparse_free(parser);

    if (fd != -1) {
        close(fd);
    }

    fclose(error_stream);

    /* If there were no errors, return NULL */
    if ((error_buffer == NULL) || (error_buffer[0] == '\0')) {
        free(error_buffer);
        return NULL;
    }

    return error_buffer;
}

static bool manpage_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    char *manpage_errors;
    bool result = true;
    const char *arch;
    char *msg = NULL;

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Is this a man page? */
    if (!file->fullpath || !S_ISREG(file->st.st_mode)) {
        return true;
    }

    if (!process_file_path(file, ri->manpage_path_include, ri->manpage_path_exclude)) {
        return true;
    }

    arch = headerGetString(file->rpm_header, RPMTAG_ARCH);

    if ((manpage_errors = inspect_manpage_validity(file->fullpath, file->localpath)) != NULL) {
        xasprintf(&msg, "Man page checker reported problems with %s on %s", file->localpath, arch);

        add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_MAN, msg, manpage_errors, REMEDY_MAN_ERRORS);

        result = false;
        free(manpage_errors);
    }

    if (!inspect_manpage_path(file->fullpath)) {
        xasprintf(&msg, "Man page %s has incorrect path on %s", file->localpath, arch);

        add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_MAN, msg, NULL, REMEDY_MAN_PATH);

        result = false;
    }

    free(msg);
    return result;
}

bool inspect_manpage(struct rpminspect *ri)
{
    bool result;

    inspect_manpage_alloc();
    result = foreach_peer_file(ri, manpage_driver);
    inspect_manpage_free();

    if (result) {
        add_result(&ri->results, RESULT_OK, NOT_WAIVABLE, HEADER_MAN, NULL, NULL, NULL);
    }

    return result;
}
