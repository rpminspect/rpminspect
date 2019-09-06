/*
 * Copyright (C) 2019  Red Hat, Inc.
 * Author(s):  David Shea <dshea@redhat.com>
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
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rpminspect.h"

struct hsearch_data * list_to_table(const string_list_t *list)
{
    struct hsearch_data *table;
    size_t table_size = 0;
    ENTRY e;
    ENTRY *eptr;

    const string_entry_t *iter;

    /* Iterate over the list once to get the size */
    table_size = list_len(list);

    if (table_size == 0) {
        table_size = 1;
    }

    /* Allocate the table */
    table = calloc(1, sizeof(*table));
    assert(table != NULL);

    if (!hcreate_r(table_size, table)) {
        fprintf(stderr, "Unable to create hash table: %s\n", strerror(errno));
        free(table);
        return NULL;
    }

    TAILQ_FOREACH(iter, list, items) {
        e.key = iter->data;
        e.data = iter->data;

        if (!hsearch_r(e, ENTER, &eptr, table)) {
            fprintf(stderr, "Unable to add entry to hash table: %s\n", strerror(errno));
            hdestroy_r(table);
            free(table);
            return NULL;
        }
    }

    return table;
}

/* Return a new list of entries that are in list a but are not in list b */
string_list_t * list_difference(const string_list_t *a, const string_list_t *b)
{
    struct hsearch_data *b_table;

    ENTRY e;
    ENTRY *eptr;

    const string_entry_t *iter;
    string_list_t *ret;
    string_entry_t *entry;

    /* Copy list b into a hash table */
    b_table = list_to_table(b);

    if (b_table == NULL) {
        return NULL;
    }

    ret = malloc(sizeof(*ret));
    assert(ret != NULL);
    TAILQ_INIT(ret);

    /* Iterate through list a looking for things not in list b */
    TAILQ_FOREACH(iter, a, items) {
        e.key = iter->data;
        hsearch_r(e, FIND, &eptr, b_table);

        if (eptr == NULL) {
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->data = iter->data;
            TAILQ_INSERT_TAIL(ret, entry, items);
        }
    }

    /* Free the hash table */
    hdestroy_r(b_table);
    free(b_table);

    return ret;
}

/* Return a new list of entries that are both in list a and list b */
string_list_t * list_intersection(const string_list_t *a, const string_list_t *b)
{
    struct hsearch_data *b_table;

    ENTRY e;
    ENTRY *eptr;

    const string_entry_t *iter;
    string_list_t *ret;
    string_entry_t *entry;

    /* Copy list b into a hash table */
    b_table = list_to_table(b);

    if (b_table == NULL) {
        return NULL;
    }

    ret = malloc(sizeof(*ret));
    assert(ret != NULL);
    TAILQ_INIT(ret);

    /* Iterate through list a looking for things in list b */
    TAILQ_FOREACH(iter, a, items) {
        e.key = iter->data;
        hsearch_r(e, FIND, &eptr, b_table);

        if (eptr != NULL) {
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->data = iter->data;
            TAILQ_INSERT_TAIL(ret, entry, items);
        }
    }

    /* Free the hash table */
    hdestroy_r(b_table);
    free(b_table);

    return ret;
}

/* Return a new list of entries that are in either list a or list b */
string_list_t * list_union(const string_list_t *a, const string_list_t *b)
{
    struct hsearch_data u_table;
    size_t u_table_size = 0;

    const string_entry_t *iter;
    string_list_t *ret;
    string_entry_t *entry;

    ENTRY e;
    ENTRY *eptr;

    ret = malloc(sizeof(*ret));
    assert(ret != NULL);
    TAILQ_INIT(ret);

    u_table_size += list_len(a);
    u_table_size += list_len(b);

    if (u_table_size == 0) {
        u_table_size = 1;
    }

    memset(&u_table, 0, sizeof(u_table));

    if (!hcreate_r(u_table_size, &u_table)) {
        fprintf(stderr, "Unable to create hash table: %s\n", strerror(errno));
        return NULL;
    }

    /*
     * Iterate over both lists, adding each entry to u_table. If it's not already in
     * u_table, add it to the list to be returned.
     */
    TAILQ_FOREACH(iter, a, items) {
        e.key = iter->data;
        e.data = iter->data;
        hsearch_r(e, FIND, &eptr, &u_table);

        if (eptr == NULL) {
            hsearch_r(e, ENTER, &eptr, &u_table);
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->data = iter->data;
            TAILQ_INSERT_TAIL(ret, entry, items);
        }
    }

    TAILQ_FOREACH(iter, b, items) {
        e.key = iter->data;
        e.data = iter->data;
        hsearch_r(e, FIND, &eptr, &u_table);

        if (eptr == NULL) {
            hsearch_r(e, ENTER, &eptr, &u_table);
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->data = iter->data;
            TAILQ_INSERT_TAIL(ret, entry, items);
        }
    }

    hdestroy_r(&u_table);

    return ret;
}

/* Return a new list of entries that in either list a or list b, but not both */
string_list_t * list_symmetric_difference(const string_list_t *a, const string_list_t *b)
{
    string_list_t *a_minus_b;
    string_list_t *b_minus_a;
    string_list_t *combination;

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

void list_free(string_list_t *list, list_entry_data_free_func free_func)
{
    string_entry_t *entry;

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
}

static int compare_entries(const void *data1, const void *data2)
{
    const string_entry_t *entry1 = (const string_entry_t *) data1;
    const string_entry_t *entry2 = (const string_entry_t *) data2;

    return strcmp(entry1->data, entry2->data);
}

/* tdestroy can't just take NULL for the free_node parameter, it has to have a no-op function */
static void do_nothing(void *nodep __attribute__((unused)))
{
    return;
}

/* Return a sorted copy of the list.
 *
 * The data pointers used by the sorted list entries are the same as those
 * used in the original list.
 */
string_list_t * list_sort(const string_list_t *list)
{
    string_entry_t *iter;
    string_list_t *sorted_list = NULL;
    void *tree = NULL;

    /* define the twalk action as a nested function so we can get to the sorted_list */
    void walk_action(const void *nodep, const VISIT which, const int depth __attribute__((unused)))
    {
        string_entry_t *entry = *((string_entry_t **) nodep);
        string_entry_t *sorted_entry;

        if ((which == postorder) || (which == leaf)) {
            sorted_entry = calloc(1, sizeof(*sorted_entry));
            assert(sorted_entry != NULL);
            sorted_entry->data = entry->data;
            TAILQ_INSERT_TAIL(sorted_list, sorted_entry, items);
        }
    }


    /* copy entries into a tree to sort */
    TAILQ_FOREACH(iter, list, items) {
        if (tsearch(iter, &tree, compare_entries) == NULL) {
            goto cleanup;
        }
    }

    /* walk the tree to create a new sorted list */
    sorted_list = malloc(sizeof(*sorted_list));
    assert(sorted_list != NULL);
    TAILQ_INIT(sorted_list);
    twalk(tree, walk_action);

cleanup:
    /* Free the tree */
    tdestroy(tree, do_nothing);

    return sorted_list;
}

size_t list_len(const string_list_t *list)
{
    string_entry_t *iter;
    size_t len = 0;

    TAILQ_FOREACH(iter, list, items) {
        len++;
    }

    return len;
}

string_list_t * list_copy(const string_list_t *list)
{
    const string_entry_t *iter;
    string_list_t *result;
    string_entry_t *entry;

    result = calloc(1, sizeof(*result));
    assert(result != NULL);
    TAILQ_INIT(result);

    TAILQ_FOREACH(iter, list, items) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);

        entry->data = strdup(iter->data);
        assert(entry->data != NULL);

        TAILQ_INSERT_TAIL(result, entry, items);
    }

    return result;
}
