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

#include "config.h"

#include <ctype.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/types.h>
#include "rpminspect.h"

/*
 * Check the given string for any defined bad words, return true if found.
 */
bool has_bad_word(const char *s, const string_list_t *badwords) {
    string_entry_t *badword = NULL;
    size_t badword_len;
    char *search;
    bool found = false;

    assert(s != NULL);

    if (badwords == NULL) {
        return false;
    }

    /* check each bad word expression for a match with our string */
    TAILQ_FOREACH(badword, badwords, items) {
        /* Do a case insensitive search for the word */
        search = strcasestr(s, badword->data);

        if (search != NULL) {
            /*
             * Only consider this a match if it's at the beginning or end of a word,
             * determined by the match being at the beginning or end of the string,
             * or preceded or followed by a space
             */
            if (search == s) {
                found = true;
                break;
            }

            /*
             * Only check for a preceding space after we're sure that "search"
             * is not the beginning of the string
             */
            if (isspace(*(search - 1))) {
                found = true;
                break;
            }

            badword_len = strlen(badword->data);
            if ((*(search + badword_len) == '\0') || isspace(*(search + badword_len))) {
                found = true;
                break;
            }
        }
    }

    return found;
}
