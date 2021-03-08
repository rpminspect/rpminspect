/*
 * Copyright © 2019 Red Hat, Inc.
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

/**
 * @file badwords.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019-2020
 * @brief Check for `bad' (unprofessional) words in strings.
 * @copyright LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include "queue.h"
#include "rpminspect.h"

/**
 * @brief Check the given string for any defined bad words, return
 * true if found.
 *
 * Given a list of bad words, check the specified string for any of
 * those bad words and return true on a match.  The search is
 * conducted with **strcasestr(3)** as well as checking for a preceeding
 * space to ensure it avoids substrings in the middle of a word.  For
 * example, if the badwords list contains `flag' then this function
 * will match ` flag' and ` flagging' but not ` conflagration'.  If
 * the list of bad words provided is empty, the function returns
 * false.
 *
 * @param s NUL-terminated string to scan for bad words.
 * @param badwords List of bad words to look for.
 * @return True if a bad word was found in the string, false otherwise.
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
