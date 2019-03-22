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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <libkmod.h>

#include "inspect.h"
#include "rpminspect.h"

/* Filter a libkmod list and convert to a string_list_t */
typedef void (*_modinfo_to_entries)(string_list_t *, const struct kmod_list *);
static string_list_t * _modinfo_to_list(const struct kmod_list *list, _modinfo_to_entries convert)
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
static void _convert_module_parameters(string_list_t *list, const struct kmod_list *modinfo)
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

void _convert_module_dependencies(string_list_t *list, const struct kmod_list *modinfo)
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
}


/* Compare two kernel modules to see if the after module lost parameters.
 *
 * The before and after lists must be module info lists returned by kmod_module_get_info.
 *
 * If after did not lose any parameters, returns true. If after lost parameters,
 * returns false, and populates "lost" with a list of the missing parameters.
 */
bool compare_module_parameters(const struct kmod_list *before, const struct kmod_list *after, string_list_t **lost)
{
    string_list_t *before_parm_list;
    string_list_t *after_parm_list;
    string_list_t *difference;

    bool result;

    assert(before);
    assert(after);
    assert(lost);

    /* Get the parameter list for each module */
    before_parm_list = _modinfo_to_list(before, _convert_module_parameters);
    after_parm_list = _modinfo_to_list(after, _convert_module_parameters);

    /* diff the parameter lists */
    difference = list_difference(before_parm_list, after_parm_list);
    assert(difference != NULL);

    /* If the list is empty, everything is fine.
     * Otherwise, make a copy of difference so we can clean everything up
     */
    if (TAILQ_EMPTY(difference)) {
        result = true;
    } else {
        result = false;
        *lost = list_copy(difference);
    }

    list_free(difference, NULL);
    list_free(before_parm_list, free);
    list_free(after_parm_list, free);

    return result;
}

/* Compare two kernel modules to see if the dependencies changed.
 *
 * Any change in dependencies is considered bad. If dependencies changed, the function
 * will return false, and the "before_deps" and "after_deps" parameters will be populated
 * with the dependencies found for the given modules.
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

    before_depends_list = _modinfo_to_list(before, _convert_module_dependencies);
    after_depends_list = _modinfo_to_list(after, _convert_module_dependencies);

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
