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

#include <sys/queue.h>
#include <sys/types.h>
#include <regex.h>
#include "rpminspect.h"

/*
 * Check the given string for any defined bad words, return true if found.
 */
bool has_bad_word(const char *s, const string_list_t *badwords) {
    string_entry_t *badexp = NULL;
    regex_t re;
    int reg_result;
    char reg_error[BUFSIZ];
    regmatch_t match[1];
    bool found = false;

    assert(s != NULL);

    if (badwords == NULL) {
        return false;
    }

    /* check each bad word expression for a match with our string */
    TAILQ_FOREACH(badexp, badwords, items) {
        /* compile this expression, POSIX extended and case insensitive */
        reg_result = regcomp(&re, badexp->data, REG_EXTENDED | REG_ICASE);

        if (reg_result != 0) {
            regerror(reg_result, &re, reg_error, sizeof(reg_error));
            fprintf(stderr, "*** Unable to compile bad word regular expression: %s\n", reg_error);
            fflush(stderr);
            return false;
        }

        /* look for a match */
        if (regexec(&re, s, 1, match, 0) == 0) {
            found = true;
        }

        /* clean up */
        regfree(&re);

        if (found) {
            break;
        }
    }

    return found;
}
