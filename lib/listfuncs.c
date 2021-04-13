/*
 * Copyright © 2019 Red Hat, Inc.
 * Author(s): David Shea <dshea@redhat.com>
 *            David Cantrell <dcantrell@redhat.com>
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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uthash.h"
#include "rpminspect.h"

string_list_t *sorted_list = NULL;

/**
 * @brief Join all members of a string_list_t in to a single string.
 *
 * Given a string_list_t, combine all the members in to a newly
 * allocated string.  An optional delimiter can be provided by passing
 * a string as the delimiter argument.  If NULL given as the
 * delimited, all strings will be concatenated together.  Caller is
 * responsible for freeing memory allocated by this function.
 *
 * @param list string_list_t containing members to join
 * @param delimiter Optional delimiter string to put between each list
 *        member (NULL to disable)
 * @return Newly allocated string of concatenated list members; caller
 *         must free.
 */
char *list_to_string(const string_list_t *list, const char *delimiter)
{
    size_t pos = 0;
    char *s = NULL;
    string_entry_t *entry = NULL;

    if (list == NULL || TAILQ_EMPTY(list)) {
        return NULL;
    }

    s = strdup("");
    assert(s != NULL);

    TAILQ_FOREACH(entry, list, items) {
        if (pos > 0 && delimiter != NULL) {
            s = strappend(s, delimiter, NULL);
        }

        s = strappend(s, entry->data, NULL);
        pos++;
    }

    return s;
}

/*
 * Lightweight conversion of a string_list_t to an array of pointers
 * to the entry->data strings.  The caller should take care to only
 * free the entry->data pointers once; either with list_free(list,
 * free) or by iterating this array and calling free() on each.  If
 * the latter, then list_free() should still be used on list, but pass
 * NULL as the free func.
 */
char **list_to_array(const string_list_t *list)
{
    int i = 0;
    char **array = NULL;
    string_entry_t *entry = NULL;

    assert(list != NULL);

    array = calloc(list_len(list), sizeof(*array));
    assert(array != NULL);

    TAILQ_FOREACH(entry, list, items) {
        array[i] = entry->data;
        i++;
    }

    return array;
}

/* Convert given string_list_t in a strhash hash table. */
string_map_t *list_to_table(const string_list_t *list)
{
    string_map_t *table = NULL;
    string_map_t *hentry = NULL;
    const string_entry_t *iter;

    if (list == NULL) {
        return NULL;
    }

    /* Allocate the table */
    TAILQ_FOREACH(iter, list, items) {
        hentry = calloc(1, sizeof(*hentry));
        assert(hentry != NULL);
        hentry->key = strdup(iter->data);
        hentry->value = strdup(iter->data);
        HASH_ADD_KEYPTR(hh, table, hentry->key, strlen(hentry->key), hentry);
    }

    return table;
}

/* Return a new list of entries that are in list a but are not in list b */
string_list_t *list_difference(const string_list_t *a, const string_list_t *b)
{
    string_map_t *b_table = NULL;
    string_map_t *hentry = NULL;
    const string_entry_t *iter = NULL;
    string_list_t *ret = NULL;
    string_entry_t *entry = NULL;

    /* Simple cases */
    if ((a == NULL || TAILQ_EMPTY(a)) && (b == NULL || TAILQ_EMPTY(b))) {
        return NULL;
    } else if ((a == NULL || TAILQ_EMPTY(a)) && (b != NULL && !TAILQ_EMPTY(b))) {
        return list_copy(b);
    } else if ((a != NULL && !TAILQ_EMPTY(a)) && (b == NULL || TAILQ_EMPTY(b))) {
        return list_copy(a);
    }

    /* Copy list b into a hash table */
    b_table = list_to_table(b);

    /* Iterate through list a looking for things not in list b */
    TAILQ_FOREACH(iter, a, items) {
        HASH_FIND_STR(b_table, iter->data, hentry);

        if (hentry == NULL) {
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->data = strdup(iter->data);

            if (ret == NULL) {
                ret = calloc(1, sizeof(*ret));
                assert(ret != NULL);
                TAILQ_INIT(ret);
            }

            TAILQ_INSERT_TAIL(ret, entry, items);
        }
    }

    /* Free the hash table */
    free_string_map(b_table);

    return ret;
}

/* Return a new list of entries that are both in list a and list b */
string_list_t *list_intersection(const string_list_t *a, const string_list_t *b)
{
    string_map_t *b_table = NULL;
    string_map_t *hentry = NULL;
    const string_entry_t *iter = NULL;
    string_list_t *ret = NULL;
    string_entry_t *entry = NULL;

    /* Copy list b into a hash table */
    b_table = list_to_table(b);

    if (b_table == NULL) {
        return NULL;
    }

    /* Iterate through list a looking for things in list b */
    TAILQ_FOREACH(iter, a, items) {
        HASH_FIND_STR(b_table, iter->data, hentry);

        if (hentry != NULL) {
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->data = strdup(iter->data);

            if (ret == NULL) {
                ret = calloc(1, sizeof(*ret));
                assert(ret != NULL);
                TAILQ_INIT(ret);
            }

            TAILQ_INSERT_TAIL(ret, entry, items);
        }
    }

    /* Free the hash table */
    free_string_map(b_table);

    return ret;
}

/* Return a new list of entries that are in either list a or list b */
string_list_t *list_union(const string_list_t *a, const string_list_t *b)
{
    string_map_t *u_table = NULL;
    string_map_t *hentry = NULL;
    const string_entry_t *iter = NULL;
    string_list_t *ret = NULL;
    string_entry_t *entry = NULL;

    /*
     * Iterate over both lists, adding each entry to u_table. If it's not already in
     * u_table, add it to the list to be returned.
     */
    TAILQ_FOREACH(iter, a, items) {
        HASH_FIND_STR(u_table, iter->data, hentry);

        if (hentry == NULL) {
            hentry = calloc(1, sizeof(*hentry));
            assert(hentry != NULL);
            hentry->key = strdup(iter->data);
            HASH_ADD_KEYPTR(hh, u_table, hentry->key, strlen(hentry->key), hentry);

            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->data = strdup(iter->data);

            if (ret == NULL) {
                ret = calloc(1, sizeof(*ret));
                assert(ret != NULL);
                TAILQ_INIT(ret);
            }

            TAILQ_INSERT_TAIL(ret, entry, items);
        }
    }

    TAILQ_FOREACH(iter, b, items) {
        HASH_FIND_STR(u_table, iter->data, hentry);

        if (hentry == NULL) {
            hentry = calloc(1, sizeof(*hentry));
            assert(hentry != NULL);
            hentry->key = strdup(iter->data);
            HASH_ADD_KEYPTR(hh, u_table, hentry->key, strlen(hentry->key), hentry);

            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->data = strdup(iter->data);

            if (ret == NULL) {
                ret = calloc(1, sizeof(*ret));
                assert(ret != NULL);
                TAILQ_INIT(ret);
            }

            TAILQ_INSERT_TAIL(ret, entry, items);
        }
    }

    free_string_map(u_table);

    return ret;
}

/* Return a new list of entries that in either list a or list b, but not both */
string_list_t * list_symmetric_difference(const string_list_t *a, const string_list_t *b)
{
    string_list_t *a_minus_b = NULL;
    string_list_t *b_minus_a = NULL;
    string_list_t *combination = NULL;

    a_minus_b = list_difference(a, b);

    if (a_minus_b == NULL) {
        return NULL;
    }

    b_minus_a = list_difference(b, a);

    if (b_minus_a == NULL) {
        list_free(a_minus_b, NULL);
        return NULL;
    }

    combination = list_union(a_minus_b, b_minus_a);

    list_free(a_minus_b, NULL);
    list_free(b_minus_a, NULL);

    return combination;
}

/*
 * Helper function to free a string_list_t and each entry->data.  If
 * the free_func is NULL, nothing is done to the entry->data values.
 */
void list_free(string_list_t *list, list_entry_data_free_func free_func)
{
    string_entry_t *entry = NULL;

    if (list == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(list)) {
        entry = TAILQ_FIRST(list);
        TAILQ_REMOVE(list, entry, items);

        if (free_func != NULL) {
            free_func(entry->data);
        }

        free(entry);
    }

    free(list);
    return;
}

static int compare_entries(string_map_t *data1, string_map_t *data2)
{
    return strcmp(data1->key, data2->key);
}

/*
 * Return a sorted copy of the list.
 *
 * The data pointers used by the sorted list entries are the same as those
 * used in the original list.
 */
string_list_t *list_sort(const string_list_t *list)
{
    string_list_t *sorted_list = NULL;
    string_entry_t *iter = NULL;
    string_map_t *map = NULL;
    string_map_t *entry = NULL;
    string_map_t *tmp_entry = NULL;

    /* create a sorted hash table of the entries */
    TAILQ_FOREACH(iter, list, items) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->key = strdup(iter->data);
        assert(entry->key != NULL);
        HASH_ADD_KEYPTR_INORDER(hh, map, &entry->key, strlen(entry->key), entry, compare_entries);
    }

    /* build a new string_list_t from the sorted hash table */
    HASH_ITER(hh, map, entry, tmp_entry) {
        HASH_DEL(map, entry);

        iter = calloc(1, sizeof(*iter));
        assert(iter != NULL);
        iter->data = strdup(entry->key);
        assert(iter->data != NULL);

        if (sorted_list == NULL) {
            sorted_list = calloc(1, sizeof(*sorted_list));
            assert(sorted_list != NULL);
            TAILQ_INIT(sorted_list);
        }

        TAILQ_INSERT_TAIL(sorted_list, iter, items);

        free(entry->key);
        free(entry);
    }

    free(map);
    return sorted_list;
}

/* Returns the number of entries in the list */
size_t list_len(const string_list_t *list)
{
    string_entry_t *iter = NULL;
    size_t len = 0;

    if (list == NULL || TAILQ_EMPTY(list)) {
        return len;
    }

    TAILQ_FOREACH(iter, list, items) {
        len++;
    }

    return len;
}

/*
 * Returns a malloc'ed copy of the given list.  Caller must use
 * list_free(list, free) on the returned list.
 */
string_list_t * list_copy(const string_list_t *list)
{
    const string_entry_t *iter = NULL;
    string_list_t *result = NULL;
    string_entry_t *entry = NULL;

    if (list == NULL) {
        return NULL;
    }

    TAILQ_FOREACH(iter, list, items) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);

        entry->data = strdup(iter->data);
        assert(entry->data != NULL);

        if (result == NULL) {
            result = calloc(1, sizeof(*result));
            assert(result != NULL);
            TAILQ_INIT(result);
        }

        TAILQ_INSERT_TAIL(result, entry, items);
    }

    return result;
}

/**
 * Given a NULL terminated array of strings, create a newly allocated
 * string_list_t containing all of the array members.  Returned the
 * new list.  Caller is responsible for freeing memory allocated by
 * this function.
 *
 * @param array NULL terminated array of strings.
 * @return Newly allocated string_list_t representing the array.
 */
string_list_t *list_from_array(const char **array)
{
    int i = 0;
    string_list_t *list = NULL;
    string_entry_t *entry = NULL;

    assert(array != NULL);

    for (i = 0; array[i] != NULL; i++) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->data = strdup(array[i]);

        if (list == NULL) {
            list = calloc(1, sizeof(*list));
            assert(list != NULL);
            TAILQ_INIT(list);
        }

        TAILQ_INSERT_TAIL(list, entry, items);
    }

    return list;
}
