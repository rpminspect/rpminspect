/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
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
#include <fnmatch.h>
#include <search.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <libkmod.h>

#include "inspect.h"
#include "rpminspect.h"

/* Filter a libkmod list and convert to a string_list_t */
static string_list_t * modinfo_to_list(const struct kmod_list *list, modinfo_to_entries convert)
{
    const struct kmod_list *iter = NULL;
    string_list_t *result;

    result = calloc(1, sizeof(*result));
    assert(result != NULL);

    TAILQ_INIT(result);

    kmod_list_foreach(iter, list) {
        convert(result, iter);
    }

    return result;
}

/* Helper for compare_parameters */
static void convert_module_parameters(string_list_t *list, const struct kmod_list *modinfo)
{
    const char *key;
    const char *value;
    char *tmp;
    string_entry_t *entry;

    key = kmod_module_info_get_key(modinfo);

    if (strcmp(key, "parm") != 0) {
        return;
    }

    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);

    /* The value is of the form <name>:<description>. Drop the description */
    value = kmod_module_info_get_value(modinfo);
    tmp = strchr(value, ':');

    if (tmp == NULL) {
        entry->data = strdup(value);
        assert(entry->data != NULL);
    } else {
        xasprintf(&entry->data, "%.*s", (int) (tmp - value), value);
    }

    TAILQ_INSERT_TAIL(list, entry, items);
}

void convert_module_dependencies(string_list_t *list, const struct kmod_list *modinfo)
{
    const char *key;
    const char *value;
    char *value_copy;
    char *value_iter;
    char *token;
    string_entry_t *entry;

    key = kmod_module_info_get_key(modinfo);

    if (strcmp(key, "depends") != 0) {
        return;
    }

    value = kmod_module_info_get_value(modinfo);

    if ((value == NULL) || (*value == '\0')) {
        return;
    }

    /* The value is a comma-separated list of dependencies. Break it up into individual entries. */
    value_copy = strdup(value);
    assert(value_copy != NULL);

    value_iter = value_copy;
    while ((token = strsep(&value_iter, ",")) != NULL) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);

        entry->data = strdup(token);
        assert(entry->data != NULL);

        TAILQ_INSERT_TAIL(list, entry, items);
    }

    free(value_copy);
    return;
}

/*
 * Compare two kernel modules to see if the after module lost
 * parameters.
 *
 * The before and after lists must be module info lists returned by
 * kmod_module_get_info.
 *
 * If after did not lose any parameters, returns true. If after lost
 * parameters, returns false, and populates "lost" with a list of the
 * missing parameters.  The "gain" list is just for informational
 * purposes and will never trigger a false return.  Only lost params
 * cause a false return.
 */
bool compare_module_parameters(const struct kmod_list *before, const struct kmod_list *after, string_list_t **lost, string_list_t **gain)
{
    string_list_t *before_parm_list;
    string_list_t *after_parm_list;
    string_list_t *difference;
    string_list_t *added;

    bool result;

    assert(before);
    assert(after);
    assert(lost);

    /* Get the parameter list for each module */
    before_parm_list = modinfo_to_list(before, convert_module_parameters);
    after_parm_list = modinfo_to_list(after, convert_module_parameters);

    /* diff the parameter lists */
    difference = list_difference(before_parm_list, after_parm_list);
    added = list_difference(after_parm_list, before_parm_list);

    /* If the lists are empty, everything is fine.
     * Otherwise, make a copy of difference so we can clean everything up
     */
    if (difference == NULL || TAILQ_EMPTY(difference)) {
        result = true;
    } else {
        result = false;
        *lost = list_copy(difference);
    }

    if (added != NULL && !TAILQ_EMPTY(added)) {
        *gain = list_copy(added);
    }

    list_free(added, NULL);
    list_free(difference, NULL);
    list_free(before_parm_list, free);
    list_free(after_parm_list, free);

    return result;
}

/*
 * Compare two kernel modules to see if the dependencies changed.
 *
 * Any change in dependencies is considered bad. If dependencies
 * changed, the function will return false, and the "before_deps" and
 * "after_deps" parameters will be populated with the dependencies
 * found for the given modules.
 */
bool compare_module_dependencies(const struct kmod_list *before, const struct kmod_list *after,
        string_list_t **before_deps, string_list_t **after_deps)
{
    string_list_t *before_depends_list;
    string_list_t *after_depends_list;
    string_list_t *difference;

    assert(before);
    assert(after);
    assert(before_deps);
    assert(after_deps);

    before_depends_list = modinfo_to_list(before, convert_module_dependencies);
    after_depends_list = modinfo_to_list(after, convert_module_dependencies);

    difference = list_symmetric_difference(before_depends_list, after_depends_list);

    /* If the list is empty, everything is fine, free everything. */
    if (TAILQ_EMPTY(difference)) {
        list_free(difference, NULL);
        list_free(before_depends_list, free);
        list_free(after_depends_list, free);

        return true;
    }

    /* Otherwise, just free the difference, and return the before and after dependencies */
    list_free(difference, NULL);

    *before_deps = before_depends_list;
    *after_deps = after_depends_list;

    return false;
}

/*
 * Add a module's alias information to a kernel_alias_data collection.
 * The returned kernel_alias_data_t must be freed with free_module_aliases.
 */
kernel_alias_data_t *gather_module_aliases(const char *module_name, const struct kmod_list *modinfo_list)
{
    kernel_alias_data_t *ret = NULL;
    const struct kmod_list *iter = NULL;
    const char *key;
    const char *value;
    struct alias_entry_t *alias_entry;

    assert(module_name);
    assert(modinfo_list);

    ret = calloc(1, sizeof(*ret));
    assert(ret);

    ret->alias_list = calloc(1, sizeof(*(ret->alias_list)));
    assert(ret->alias_list);

    TAILQ_INIT(ret->alias_list);

    kmod_list_foreach(iter, modinfo_list) {
        /* Only gather pci aliases */
        key = kmod_module_info_get_key(iter);

        if (strcmp(key, "alias") != 0) {
            continue;
        }

        value = kmod_module_info_get_value(iter);

        if (!strprefix(value, "pci:")) {
            continue;
        }

        alias_entry = calloc(1, sizeof(*alias_entry));
        assert(alias_entry);

        alias_entry->alias = strdup(value);
        assert(alias_entry->alias);

        alias_entry->module = strdup(module_name);
        assert(alias_entry->module);

        ret->num_aliases++;
        TAILQ_INSERT_TAIL(ret->alias_list, alias_entry, items);
    }

    return ret;
}

/* Free the kernel_alias_data struct created by gather_module_aliases */
void free_module_aliases(kernel_alias_data_t *data)
{
    struct alias_entry_t *alias_entry;
    ENTRY e;
    ENTRY *eptr;

    if (data == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(data->alias_list)) {
        alias_entry = TAILQ_FIRST(data->alias_list);
        TAILQ_REMOVE(data->alias_list, alias_entry, items);

        if (data->alias_table != NULL) {
            e.key = alias_entry->alias;
            hsearch_r(e, FIND, &eptr, data->alias_table);

            if ((eptr != NULL) && (eptr->data != NULL)) {
                list_free((string_list_t *) eptr->data, free);
                eptr->data = NULL;
            }
        }

        free(alias_entry->alias);
        free(alias_entry->module);
        free(alias_entry);
    }

    free(data->alias_list);

    if (data->alias_table != NULL) {
        hdestroy_r(data->alias_table);
        free(data->alias_table);
    }

    free(data);
    return;
}

static void finalize_module_aliases(kernel_alias_data_t *data)
{
    struct alias_entry_t *alias_entry;
    struct alias_entry_t *tmp;
    ENTRY e;
    ENTRY *eptr;
    string_entry_t *module_entry;
    int result;

    /* ignore unused variable warnings if assert is disabled */
    (void) result;

    assert(data);

    data->alias_table = calloc(1, sizeof(*(data->alias_table)));
    assert(data->alias_table);

    /*
     * The glibc man page for hcreate recommends a size 25% greater than
     * the number of elements, so use that
     */
    result = hcreate_r(data->num_aliases * 1.25, data->alias_table);
    assert(result != 0);

    alias_entry = TAILQ_FIRST(data->alias_list);
    while (alias_entry != NULL) {
        /* Create a new entry for the alias -> module-list mapping */
        module_entry = calloc(1, sizeof(*module_entry));
        assert(module_entry);

        module_entry->data = strdup(alias_entry->module);
        assert(module_entry->data);

        /* Find or insert the alias */
        e.key = alias_entry->alias;
        e.data = NULL;
        result = hsearch_r(e, ENTER, &eptr, data->alias_table);
        assert(result != 0);

        if (eptr->data != NULL) {
            /* The alias was already in the table. First, add this module name to the list in "data" */
            TAILQ_INSERT_TAIL((string_list_t *) eptr->data, module_entry, items);

            /* Now delete this duplicate alias entry from alias_list */
            tmp = TAILQ_NEXT(alias_entry, items);
            TAILQ_REMOVE(data->alias_list, alias_entry, items);

            free(alias_entry->alias);
            free(alias_entry->module);
            free(alias_entry);

            /* Advance past the deleted entry */
            alias_entry = tmp;
        } else {
            /* The alias was not in the table, start a new module name list */
            eptr->data = calloc(1, sizeof(string_list_t));
            assert(eptr->data);

            TAILQ_INIT((string_list_t *) eptr->data);
            TAILQ_INSERT_TAIL((string_list_t *) eptr->data, module_entry, items);

            /* Advance to the next entry */
            alias_entry = TAILQ_NEXT(alias_entry, items);
        }
    }

    return;
}

static string_list_t * wildcard_alias_search(const char *alias, const struct alias_list_t *after_aliases)
{
    string_list_t *result;
    string_entry_t *entry;
    struct alias_entry_t *iter;

    result = calloc(1, sizeof(*result));
    assert(result);
    TAILQ_INIT(result);

    TAILQ_FOREACH(iter, after_aliases, items) {
        if (fnmatch(iter->alias, alias, 0) == 0) {
            entry = calloc(1, sizeof(*entry));
            assert(entry);

            entry->data = strdup(iter->module);
            assert(entry->data);

            TAILQ_INSERT_TAIL(result, entry, items);
        }
    }

    return result;
}

/*
 * For each module alias in "before", ensure that the alias is
 * provided by the same modules in "after".  If "after" lost
 * providers, call the provided callback with the before and after
 * lists, and return false.
 *
 * Module aliases use glob-style wildcards, so not all changes in
 * strings are a regression. For example, in 2.6.25, cxgb3 changed all
 * of its sub-device values to '*', so
 * "pci:v00001425d00000020sv*sd00000001bc*sc*i*" became
 * "pci:v00001425d00000020sv*sd*bc*sc*i*". The after string still
 * matches the before string, so this is not a regression.
 *
 * However, since matching up module aliases does involve
 * arbitrary-length wildcards, this function effectively needs to run
 * fnmatch() between every combination of before and after aliases,
 * resulting in a complexity of O(n^2).  To speed things up in the
 * (hopefully) usual case, the wildcard search is only run when an
 * exact string match of an alias (using hash tables) results in an
 * apparent regression.
 */
bool compare_module_aliases(kernel_alias_data_t *before, kernel_alias_data_t *after, module_alias_callback callback, void *user_data)
{
    struct alias_entry_t *iter;
    string_list_t *before_modules;
    string_list_t *after_modules;
    string_list_t *difference;
    string_list_t empty;
    ENTRY e;
    ENTRY *eptr;
    bool wildcard_search;
    bool result = true;

    assert(callback);

    /* Before empty, nothing to check for */
    if (before == NULL) {
        return true;
    }

    /* If after is NULL, fake the results with an empty list */
    if (after == NULL) {
        TAILQ_INIT(&empty);
    }

    /* Gather the lists of aliases into hash tables, mapping an alias string to a string_list_t of module names. */
    finalize_module_aliases(before);

    if (after != NULL) {
        finalize_module_aliases(after);
    }

    /* For each alias in before, look for the matching alias in after */
    TAILQ_FOREACH(iter, before->alias_list, items) {
        e.key = iter->alias;
        hsearch_r(e, FIND, &eptr, before->alias_table);
        assert(eptr != NULL);
        before_modules = (string_list_t *) eptr->data;

        /*
         * If the after list was NULL, no need to do a wildcard search. Just call the
         * function with an empty after list.
         */
        if (after == NULL) {
            callback(iter->alias, before_modules, &empty, user_data);
            continue;
        }

        hsearch_r(e, FIND, &eptr, after->alias_table);
        wildcard_search = false;

        /* No match found, do a wildcard search */
        if (eptr == NULL) {
            after_modules = wildcard_alias_search(iter->alias, after->alias_list);
            wildcard_search = true;
        } else {
            after_modules = (string_list_t *) eptr->data;
            difference = list_difference(before_modules, after_modules);

            /* If the lists differ, do a wildcard search */
            if (difference != NULL && !TAILQ_EMPTY(difference)) {
                after_modules = wildcard_alias_search(iter->alias, after->alias_list);
                wildcard_search = true;
            }

            list_free(difference, NULL);
        }

        /* Compare the results */
        difference = list_difference(before_modules, after_modules);

        if (difference != NULL && !TAILQ_EMPTY(difference)) {
            callback(iter->alias, before_modules, after_modules, user_data);
            result = false;
        }

        /* If after_modules was created from a wildcard search, free it */
        if (wildcard_search) {
            list_free(after_modules, NULL);
        }
    }

    return result;
}
