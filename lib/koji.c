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

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <err.h>
#include <limits.h>
#include <xmlrpc-c/client.h>
#include <xmlrpc-c/client_global.h>
#include "queue.h"
#include "rpminspect.h"

/*
 * General error handler for xmlrpc failures.  Could be improved
 * a bit to be more helpful to the user.
 */
static void xmlrpc_abort_on_fault(xmlrpc_env *env)
{
    if (env->fault_occurred) {
        errx(RI_PROGRAM_ERROR, _("XML-RPC Fault: %s (%d)"), env->fault_string, env->fault_code);
    }
}

/*
 * Read a koji task info struct and store in the struct koji_task.
 */
static void read_koji_task_struct(xmlrpc_env *env, xmlrpc_value *result, struct koji_task *task)
{
    int i = 0;
    int size = 0;
    char *key = NULL;
    xmlrpc_value *k = NULL;
    xmlrpc_value *value = NULL;

    assert(env != NULL);
    assert(result != NULL);
    assert(task != NULL);

    /* read the values from the result */
    size = xmlrpc_struct_size(env, result);
    xmlrpc_abort_on_fault(env);

    for (i = 0; i < size; i++) {
        xmlrpc_struct_read_member(env, result, i, &k, &value);
        xmlrpc_abort_on_fault(env);

        /* Get the key as a string */
        xmlrpc_decompose_value(env, k, "s", &key);
        xmlrpc_abort_on_fault(env);
        xmlrpc_DECREF(k);

        /*
         * If the value of a key is nil, just skip over it.  We can't
         * do anything with nil values, so it might as well just not
         * be present in the output.
         */
        if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
            xmlrpc_DECREF(value);
            continue;
        }

        /*
         * Walk through the keys and fill in the struct.
         * This is tedious, but I would rather do it now rather
         * than unpacking XMLRPC results in later functions.
         */
        if (!strcmp(key, "weight")) {
            xmlrpc_decompose_value(env, value, "d", &task->weight);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "parent") && xmlrpc_value_type(value) != XMLRPC_TYPE_NIL) {
            xmlrpc_decompose_value(env, value, "i", &task->parent);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "completion_time")) {
            xmlrpc_decompose_value(env, value, "s", &task->completion_time);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "start_time")) {
            xmlrpc_decompose_value(env, value, "s", &task->start_time);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "start_ts")) {
            xmlrpc_decompose_value(env, value, "d", &task->start_ts);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "waiting")) {
            xmlrpc_decompose_value(env, value, "b", &task->waiting);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "awaited")) {
            xmlrpc_decompose_value(env, value, "b", &task->awaited);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "label")) {
            xmlrpc_decompose_value(env, value, "s", &task->label);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "priority")) {
            xmlrpc_decompose_value(env, value, "i", &task->priority);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "channel_id")) {
            xmlrpc_decompose_value(env, value, "i", &task->channel_id);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "state")) {
            xmlrpc_decompose_value(env, value, "i", &task->state);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "create_time")) {
            xmlrpc_decompose_value(env, value, "s", &task->create_time);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "create_ts")) {
            xmlrpc_decompose_value(env, value, "d", &task->create_ts);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "owner")) {
            xmlrpc_decompose_value(env, value, "i", &task->owner);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "host_id")) {
            xmlrpc_decompose_value(env, value, "i", &task->host_id);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "method")) {
            xmlrpc_decompose_value(env, value, "s", &task->method);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "completion_ts")) {
            xmlrpc_decompose_value(env, value, "d", &task->completion_ts);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "arch")) {
            xmlrpc_decompose_value(env, value, "s", &task->arch);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "id")) {
            xmlrpc_decompose_value(env, value, "i", &task->id);
            xmlrpc_abort_on_fault(env);
        }

        xmlrpc_DECREF(value);
        free(key);
        key = NULL;
    }

    return;
}

/*
 * Turn the array of strings in to a string_list_t.  Used when reading task
 * results through get_koji_task().
 */
static string_list_t *read_koji_descendent_results(xmlrpc_env *env, xmlrpc_value *value)
{
    int i = 0;
    int size = 0;
    string_list_t *results = NULL;
    string_entry_t *entry = NULL;
    xmlrpc_value *s = NULL;

    assert(env != NULL);
    assert(value != NULL);

    size = xmlrpc_array_size(env, value);
    xmlrpc_abort_on_fault(env);

    results = calloc(1, sizeof(*results));
    assert(results != NULL);
    TAILQ_INIT(results);

    for (i = 0; i < size; i++) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        xmlrpc_array_read_item(env, value, i, &s);
        xmlrpc_abort_on_fault(env);
        xmlrpc_decompose_value(env, s, "s", &entry->data);
        xmlrpc_abort_on_fault(env);
        xmlrpc_DECREF(s);
        TAILQ_INSERT_TAIL(results, entry, items);
    }

    return results;
}

/*
 * Initialize a koji_buildlist_t.
 */
koji_buildlist_t *init_koji_buildlist(void)
{
    koji_buildlist_t *builds = NULL;

    builds = calloc(1, sizeof(*(builds)));
    assert(builds != NULL);
    TAILQ_INIT(builds);
    return builds;
}

/*
 * Free memory associated with a koji_buildlist_t.
 */
void free_koji_buildlist(koji_buildlist_t *builds)
{
    koji_buildlist_entry_t *entry = NULL;

    if (builds == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(builds)) {
        entry = TAILQ_FIRST(builds);
        TAILQ_REMOVE(builds, entry, builditems);

        free(entry->package_name);
        entry->package_name = NULL;
        free(entry->owner_name);
        entry->owner_name = NULL;
        free(entry->nvr);
        entry->nvr = NULL;
        free(entry->start_time);
        entry->start_time = NULL;
        free(entry->creation_time);
        entry->creation_time = NULL;
        free(entry->epoch);
        entry->epoch = NULL;
        free(entry->completion_time);
        entry->completion_time = NULL;
        free(entry->tag_name);
        entry->tag_name = NULL;
        free(entry->version);
        entry->version = NULL;
        free(entry->release);
        entry->release = NULL;
        free(entry->volume_name);
        entry->volume_name = NULL;
        free(entry->name);
        entry->name = NULL;

        free_koji_rpmlist(entry->rpms);

        free(entry);
    }

    free(builds);

    return;
}

/*
 * Initialize a koji_rpmlist_t.
 */
koji_rpmlist_t *init_koji_rpmlist(void)
{
    koji_rpmlist_t *rpms = NULL;

    rpms = calloc(1, sizeof(*(rpms)));
    assert(rpms != NULL);
    TAILQ_INIT(rpms);
    return rpms;
}

/*
 * Free memory associated with a koji_rpmlist_t.
 */
void free_koji_rpmlist(koji_rpmlist_t *rpms)
{
    koji_rpmlist_entry_t *entry = NULL;

    if (rpms == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(rpms)) {
        entry = TAILQ_FIRST(rpms);
        TAILQ_REMOVE(rpms, entry, items);
        free(entry->arch);
        entry->arch = NULL;
        free(entry->name);
        entry->name = NULL;
        free(entry->version);
        entry->version = NULL;
        free(entry->release);
        entry->release = NULL;
        free(entry);
    }

    free(rpms);

    return;
}

/*
 * Initialize a struct koji_build.
 */
void init_koji_build(struct koji_build *build)
{
    assert(build != NULL);

    build->package_name = NULL;
    build->epoch = 0;
    build->name = NULL;
    build->version = NULL;
    build->release = NULL;
    build->nvr = NULL;

    build->source = NULL;

    build->creation_time = NULL;
    build->completion_time = NULL;
    build->package_id = -1;
    build->id = -1;
    build->state = -1;
    build->completion_ts = 0;
    build->owner_id = -1;
    build->owner_name = NULL;
    build->start_time = NULL;
    build->creation_event_id = -1;
    build->start_ts = 0;
    build->creation_ts = 0;
    build->task_id = -1;

    build->volume_id = -1;
    build->volume_name = NULL;

    build->original_url = NULL;

    build->modulemd_str = NULL;
    build->module_name = NULL;
    build->module_stream = NULL;
    build->module_build_service_id = -1;
    build->module_version = NULL;
    build->module_context = NULL;
    build->module_content_koji_tag = NULL;

    build->builds = init_koji_buildlist();

    return;
}

/*
 * Initialize a struct koji_task
 */
void init_koji_task(struct koji_task *task)
{
    assert(task != NULL);

    task->weight = -1;
    task->completion_time = NULL;
    task->start_time = NULL;
    task->start_ts = -1;
    task->waiting = false;
    task->awaited = false;
    task->label = NULL;
    task->priority = -1;
    task->channel_id = -1;
    task->state = -1;
    task->create_time = NULL;
    task->create_ts = -1;
    task->owner = -1;
    task->host_id = -1;
    task->method = NULL;
    task->completion_ts = -1;
    task->arch = NULL;
    task->id = -1;
    task->descendents = NULL;

    return;
}

/*
 * Initialize a struct koji_task_list
 */
void init_koji_task_entry(koji_task_entry_t *entry)
{
    assert(entry != NULL);

    entry->task = malloc(sizeof(*entry->task));
    assert(entry->task != NULL);
    init_koji_task(entry->task);

    entry->brootid = -1;

    entry->srpms = malloc(sizeof(*entry->srpms));
    assert(entry->srpms != NULL);
    TAILQ_INIT(entry->srpms);

    entry->rpms = malloc(sizeof(*entry->rpms));
    assert(entry->rpms != NULL);
    TAILQ_INIT(entry->rpms);

    entry->logs = malloc(sizeof(*entry->logs));
    assert(entry->logs != NULL);
    TAILQ_INIT(entry->logs);

    return;
}

/*
 * Free dynamically allocated memory associated with a struct koji_build.
 */
void free_koji_build(struct koji_build *build)
{
    if (build == NULL) {
        return;
    }

    free(build->package_name);
    free(build->name);
    free(build->version);
    free(build->release);
    free(build->nvr);

    free(build->source);

    free(build->creation_time);
    free(build->completion_time);
    free(build->owner_name);
    free(build->start_time);

    free(build->volume_name);

    free(build->original_url);

    free(build->modulemd_str);
    free(build->module_name);
    free(build->module_stream);
    free(build->module_version);
    free(build->module_context);
    free(build->module_content_koji_tag);

    free_koji_buildlist(build->builds);

    free(build);

    return;
}

/*
 * Free dynamically allocated memory associated with a koji_task_entry_t.
 */
void free_koji_task_entry(koji_task_entry_t *entry)
{
    if (entry == NULL) {
        return;
    }

    free_koji_task(entry->task);
    list_free(entry->srpms, free);
    list_free(entry->rpms, free);
    list_free(entry->logs, free);
    free(entry);

    return;
}

/*
 * Free dynamically allocated memory associated with a struct koji_task.
 */
void free_koji_task(struct koji_task *task)
{
    koji_task_entry_t *descendent = NULL;

    if (task == NULL) {
        return;
    }

    free(task->completion_time);
    free(task->start_time);
    free(task->label);
    free(task->create_time);
    free(task->method);
    free(task->arch);

    if (task->descendents) {
        while (!TAILQ_EMPTY(task->descendents)) {
            descendent = TAILQ_FIRST(task->descendents);
            TAILQ_REMOVE(task->descendents, descendent, items);
            free_koji_task_entry(descendent);
        }

        free(task->descendents);
    }

    free(task);

    return;
}

/*
 * Look up a Koji build and return the information in a struct koji_build.
 */
struct koji_build *get_koji_build(struct rpminspect *ri, const char *buildspec)
{
    struct koji_build *build = NULL;
    int i = 0;
    int j = 0;
    int size = 0;
    int s_sz = 0;
    xmlrpc_env env;
    xmlrpc_value *result = NULL;
    xmlrpc_value *element = NULL;
    xmlrpc_value *k = NULL;
    xmlrpc_value *value = NULL;
    char *key = NULL;
    koji_buildlist_entry_t *buildentry = NULL;
    koji_rpmlist_entry_t *rpm = NULL;

    assert(ri != NULL);

    if (buildspec == NULL) {
        return NULL;
    }

    /* if there is no koji system specified in the configuration, stop */
    if (ri->kojihub == NULL) {
        return NULL;
    }

    /* initialize everything and get XMLRPC ready */
    build = calloc(1, sizeof(*build));
    assert(build != NULL);
    init_koji_build(build);
    xmlrpc_env_init(&env);
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, SOFTWARE_NAME, PACKAGE_VERSION, NULL, 0);
    xmlrpc_abort_on_fault(&env);

    /* increase the message response size */
    xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, INT_MAX);

    /* call 'getBuild' on the koji hub */
    result = xmlrpc_client_call(&env, ri->kojihub, "getBuild", "(s)", buildspec);

    if (env.fault_occurred && env.fault_code >= 1000) {
        /* server side error which means Koji protocol error */
        xmlrpc_env_clean(&env);
        xmlrpc_client_cleanup();
        free_koji_build(build);
        return NULL;
    } else {
        xmlrpc_abort_on_fault(&env);
    }

    /* is this a valid build? */
    if (xmlrpc_value_type(result) == XMLRPC_TYPE_NIL) {
        xmlrpc_DECREF(result);
        xmlrpc_env_clean(&env);
        xmlrpc_client_cleanup();
        free_koji_build(build);
        return NULL;
    }

    /* read the values from the result */
    size = xmlrpc_struct_size(&env, result);
    xmlrpc_abort_on_fault(&env);

    for (i = 0; i < size; i++) {
        xmlrpc_struct_read_member(&env, result, i, &k, &value);
        xmlrpc_abort_on_fault(&env);

        /* Get the key as a string */
        xmlrpc_decompose_value(&env, k, "s", &key);
        xmlrpc_abort_on_fault(&env);
        xmlrpc_DECREF(k);

        /*
         * If the value of a key is nil, just skip over it.  We can't
         * do anything with nil values, so it might as well just not
         * be present in the output.
         */
        if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
            xmlrpc_DECREF(value);
            continue;
        }

        /*
         * Walk through the keys and fill in the struct.
         * This is tedious, but I would rather do it now rather
         * than unpacking XMLRPC results in later functions.
         */
        if (!strcmp(key, "package_name")) {
            xmlrpc_decompose_value(&env, value, "s", &build->package_name);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "epoch")) {
            xmlrpc_decompose_value(&env, value, "i", &build->epoch);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "name")) {
            xmlrpc_decompose_value(&env, value, "s", &build->name);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "version")) {
            xmlrpc_decompose_value(&env, value, "s", &build->version);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "release")) {
            xmlrpc_decompose_value(&env, value, "s", &build->release);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "nvr")) {
            xmlrpc_decompose_value(&env, value, "s", &build->nvr);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "source")) {
            xmlrpc_decompose_value(&env, value, "s", &build->source);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "creation_time")) {
            xmlrpc_decompose_value(&env, value, "s", &build->creation_time);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "completion_time")) {
            xmlrpc_decompose_value(&env, value, "s", &build->completion_time);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "package_id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->package_id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "build_id")) {
            /* we hit this on regular packages, modules handled below */
            buildentry = calloc(1, sizeof(*buildentry));
            assert(buildentry != NULL);

            xmlrpc_decompose_value(&env, value, "i", &buildentry->build_id);
            xmlrpc_abort_on_fault(&env);

            if (build->package_name != NULL) {
                buildentry->package_name = strdup(build->package_name);
            }

            buildentry->rpms = calloc(1, sizeof(*buildentry->rpms));
            assert(buildentry->rpms != NULL);
            TAILQ_INIT(buildentry->rpms);

            TAILQ_INSERT_TAIL(build->builds, buildentry, builditems);
        } else if (!strcmp(key, "state")) {
            xmlrpc_decompose_value(&env, value, "i", &build->state);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "completion_ts")) {
            xmlrpc_decompose_value(&env, value, "d", &build->completion_ts);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "owner_id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->owner_id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "owner_name")) {
            xmlrpc_decompose_value(&env, value, "s", &build->owner_name);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "start_time")) {
            xmlrpc_decompose_value(&env, value, "s", &build->start_time);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "creation_event_id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->creation_event_id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "start_ts")) {
            xmlrpc_decompose_value(&env, value, "d", &build->start_ts);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "volume_id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->volume_id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "creation_ts")) {
            xmlrpc_decompose_value(&env, value, "d", &build->creation_ts);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "task_id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->task_id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "volume_name")) {
            xmlrpc_decompose_value(&env, value, "s", &build->volume_name);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "extra")) {
            /*
             * EXTRA METADATA HANDLING
             * This is where module metadata hides, but there can also
             * be some stuff for regular builds.  Handle accordingly
             * and collect the information.
             */

            int subsize, j;
            xmlrpc_value *subk = NULL;
            xmlrpc_value *subv = NULL;
            char *subkey = NULL;

            /* read the values from the result */
            subsize = xmlrpc_struct_size(&env, value);
            xmlrpc_abort_on_fault(&env);
            j = 0;

            while (j < subsize) {
                xmlrpc_struct_read_member(&env, value, j, &subk, &subv);
                xmlrpc_abort_on_fault(&env);

                /* Get the key as a string */
                xmlrpc_decompose_value(&env, subk, "s", &subkey);
                xmlrpc_abort_on_fault(&env);
                xmlrpc_DECREF(subk);

                /* Skip nil values */
                if (xmlrpc_value_type(subv) == XMLRPC_TYPE_NIL) {
                    xmlrpc_DECREF(subv);
                    continue;
                }

                if (!strcmp(subkey, "source") || !strcmp(subkey, "typeinfo") || !strcmp(subkey, "module")) {
                    subsize = xmlrpc_struct_size(&env, subv);
                    xmlrpc_abort_on_fault(&env);
                    xmlrpc_DECREF(value);
                    free(subkey);
                    value = subv;
                    j = 0;
                    continue;
                } else if (!strcmp(subkey, "original_url")) {
                    xmlrpc_decompose_value(&env, subv, "s", &build->original_url);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(subkey, "modulemd_str")) {
                    xmlrpc_decompose_value(&env, subv, "s", &build->modulemd_str);
                    xmlrpc_abort_on_fault(&env);
                    ri->buildtype = KOJI_BUILD_MODULE;
                } else if(!strcmp(subkey, "name")) {
                    xmlrpc_decompose_value(&env, subv, "s", &build->module_name);
                    xmlrpc_abort_on_fault(&env);
                } else if(!strcmp(subkey, "stream")) {
                    xmlrpc_decompose_value(&env, subv, "s", &build->module_stream);
                    xmlrpc_abort_on_fault(&env);
                } else if(!strcmp(subkey, "module_build_service_id")) {
                    xmlrpc_decompose_value(&env, subv, "i", &build->module_build_service_id);
                    xmlrpc_abort_on_fault(&env);
                } else if(!strcmp(subkey, "version")) {
                    xmlrpc_decompose_value(&env, subv, "s", &build->module_version);
                    xmlrpc_abort_on_fault(&env);
                } else if(!strcmp(subkey, "context")) {
                    xmlrpc_decompose_value(&env, subv, "s", &build->module_context);
                    xmlrpc_abort_on_fault(&env);
                } else if(!strcmp(subkey, "content_koji_tag")) {
                    xmlrpc_decompose_value(&env, subv, "s", &build->module_content_koji_tag);
                    xmlrpc_abort_on_fault(&env);
                }

                j++;

                xmlrpc_DECREF(subv);
                free(subkey);
            }
        }
    }

    /* Modules have multiple builds, so collect the IDs */
    if (ri->buildtype == KOJI_BUILD_MODULE) {
        xmlrpc_DECREF(result);
        result = xmlrpc_client_call(&env, ri->kojihub, "getLatestBuilds", "(s)", build->module_content_koji_tag);
        xmlrpc_abort_on_fault(&env);

        /* read the values from the result */
        size = xmlrpc_array_size(&env, result);
        xmlrpc_abort_on_fault(&env);

        for (i = 0; i < size; i++) {
            xmlrpc_array_read_item(&env, result, i, &element);
            xmlrpc_abort_on_fault(&env);

            /* each array element is a struct */
            s_sz = xmlrpc_struct_size(&env, element);
            xmlrpc_abort_on_fault(&env);

            buildentry = calloc(1, sizeof(*buildentry));
            assert(buildentry != NULL);

            for (j = 0; j < s_sz; j++) {
                xmlrpc_struct_read_member(&env, element, j, &k, &value);
                xmlrpc_abort_on_fault(&env);

                /* Get the key as a string */
                xmlrpc_decompose_value(&env, k, "s", &key);
                xmlrpc_abort_on_fault(&env);
                xmlrpc_DECREF(k);

                /* Skip nil values */
                if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
                    xmlrpc_DECREF(value);
                    continue;
                }

                /* Grab the values we need */
                if (!strcmp(key, "build_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->build_id);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "package_name")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->package_name);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "owner_name")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->owner_name);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "task_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->task_id);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "state")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->state);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "nvr")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->nvr);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "start_time")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->start_time);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "creation_event_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->creation_event_id);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "creation_time")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->creation_time);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "epoch")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->epoch);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "tag_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->tag_id);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "completion_time")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->completion_time);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "tag_name")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->tag_name);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "version")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->version);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "volume_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->volume_id);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "release")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->release);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "package_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->package_id);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "owner_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->owner_id);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->id);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "s")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->volume_name);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "s")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->name);
                    xmlrpc_abort_on_fault(&env);
                }

                xmlrpc_DECREF(value);
                free(key);
            }

            xmlrpc_DECREF(element);

            buildentry->rpms = calloc(1, sizeof(*buildentry->rpms));
            assert(buildentry->rpms != NULL);
            TAILQ_INIT(buildentry->rpms);

            TAILQ_INSERT_TAIL(build->builds, buildentry, builditems);
        }
    }

    xmlrpc_DECREF(result);

    /* Call 'listBuildRPMs' on the koji hub for each build_id */
    TAILQ_FOREACH(buildentry, build->builds, builditems) {
        result = xmlrpc_client_call(&env, ri->kojihub, "listBuildRPMs", "(i)", buildentry->build_id);
        xmlrpc_abort_on_fault(&env);

        /* read the values from the result */
        size = xmlrpc_array_size(&env, result);
        xmlrpc_abort_on_fault(&env);

        for (i = 0; i < size; i++) {
            xmlrpc_array_read_item(&env, result, i, &element);
            xmlrpc_abort_on_fault(&env);

            /* each array element is a struct */
            s_sz = xmlrpc_struct_size(&env, element);
            xmlrpc_abort_on_fault(&env);

            /* create a new rpm list entry */
            rpm = calloc(1, sizeof(*rpm));
            assert(rpm != NULL);

            for (j = 0; j < s_sz; j++) {
                xmlrpc_struct_read_member(&env, element, j, &k, &value);
                xmlrpc_abort_on_fault(&env);

                /* Get the key as a string */
                xmlrpc_decompose_value(&env, k, "s", &key);
                xmlrpc_abort_on_fault(&env);

                /* Skip nil values */
                if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
                    xmlrpc_DECREF(value);
                    xmlrpc_DECREF(k);
                    continue;
                }

                /* Grab the values we need */
                if (!strcmp(key, "arch")) {
                    xmlrpc_decompose_value(&env, value, "s", &rpm->arch);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "name")) {
                    xmlrpc_decompose_value(&env, value, "s", &rpm->name);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "version")) {
                    xmlrpc_decompose_value(&env, value, "s", &rpm->version);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "release")) {
                    xmlrpc_decompose_value(&env, value, "s", &rpm->release);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "epoch")) {
                    xmlrpc_decompose_value(&env, value, "i", &rpm->epoch);
                } else if (!strcmp(key, "size")) {
                    if (xmlrpc_value_type(value) == XMLRPC_TYPE_INT) {
                        xmlrpc_decompose_value(&env, value, "i", &rpm->size);
                    } else if (xmlrpc_value_type(value) == XMLRPC_TYPE_I8) {
                        xmlrpc_decompose_value(&env, value, "I", &rpm->size);
                    } else {
                        /*
                         * XXX: have no idea what we got back here, set it
                         * negative for future debugging
                         */
                        rpm->size = -47;
                    }
                }

                xmlrpc_DECREF(value);
                xmlrpc_DECREF(k);
                free(key);
                key = NULL;
            }

            /* add this rpm to the list */
            if (allowed_arch(ri, rpm->arch)) {
                TAILQ_INSERT_TAIL(buildentry->rpms, rpm, items);
            }
        }
    }

    /* Cleanup */
    xmlrpc_DECREF(result);

    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();

    return build;
}

/*
 * Look up a Koji task and return the information in a struct koji_build.
 */
struct koji_task *get_koji_task(struct rpminspect *ri, const char *taskspec)
{
    int i, j, k;
    int size, dsize, rsize;
    xmlrpc_env env;
    xmlrpc_value *result = NULL;
    xmlrpc_value *xk = NULL;
    xmlrpc_value *xv = NULL;
    xmlrpc_value *tr_k = NULL;
    xmlrpc_value *tr_v = NULL;
    xmlrpc_value *dstruct = NULL;
    xmlrpc_value *dresult = NULL;
    char *key = NULL;
    struct koji_task *task = NULL;
    koji_task_entry_t *descendent = NULL;

    assert(ri != NULL);

    if (taskspec == NULL) {
        return NULL;
    }

    /* if there is no koji system specified in the configuration, stop */
    if (ri->kojihub == NULL) {
        return NULL;
    }

    /* initialize everything and get XMLRPC ready */
    task = calloc(1, sizeof(*task));
    assert(task != NULL);
    init_koji_task(task);
    xmlrpc_env_init(&env);
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, SOFTWARE_NAME, PACKAGE_VERSION, NULL, 0);
    xmlrpc_abort_on_fault(&env);

    /* increase the message response size */
    xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, INT_MAX);

    /* call 'getTaskInfo' on the koji hub */
    result = xmlrpc_client_call(&env, ri->kojihub, "getTaskInfo", "(s)", taskspec);
    xmlrpc_abort_on_fault(&env);

    /* is this a valid build? */
    if (xmlrpc_value_type(result) == XMLRPC_TYPE_NIL) {
        xmlrpc_DECREF(result);
        xmlrpc_env_clean(&env);
        xmlrpc_client_cleanup();
        return NULL;
    }

    read_koji_task_struct(&env, result, task);
    xmlrpc_DECREF(result);

    /* call 'getTaskDescendents' on the task ID */
    result = xmlrpc_client_call(&env, ri->kojihub, "getTaskDescendents", "(s)", taskspec);
    xmlrpc_abort_on_fault(&env);

    /* read the values from the result */
    size = xmlrpc_struct_size(&env, result);
    xmlrpc_abort_on_fault(&env);

    task->descendents = calloc(1, sizeof(*task->descendents));
    assert(task->descendents != NULL);
    TAILQ_INIT(task->descendents);

    for (i = 0; i < size; i++) {
        xmlrpc_struct_read_member(&env, result, i, &xk, &xv);
        xmlrpc_abort_on_fault(&env);

        /* Continue if the value is empty */
        dsize = xmlrpc_array_size(&env, xv);

        if (dsize == 0) {
            continue;
        }

        /* Read the value array */
        for (j = 0; j < dsize; j++) {
            /* get the descendent task struct */
            xmlrpc_array_read_item(&env, xv, j, &dstruct);
            xmlrpc_abort_on_fault(&env);

            /* initialize a struct and read the results */
            descendent = calloc(1, sizeof(*descendent));
            assert(descendent != NULL);
            init_koji_task_entry(descendent);
            read_koji_task_struct(&env, dstruct, descendent->task);

            /* gather the task results */
            dresult = xmlrpc_client_call(&env, ri->kojihub, "getTaskResult", "(i)", descendent->task->id);
            xmlrpc_abort_on_fault(&env);

            if (xmlrpc_value_type(dresult) == XMLRPC_TYPE_NIL) {
                /* some task IDs may be nothing, so ignore */
                xmlrpc_DECREF(dresult);
                continue;
            }

            rsize = xmlrpc_struct_size(&env, dresult);

            for (k = 0; k < rsize; k++) {
                /* Read the result struct */
                xmlrpc_struct_read_member(&env, dresult, k, &tr_k, &tr_v);
                xmlrpc_abort_on_fault(&env);

                /* Get the key as a string */
                xmlrpc_decompose_value(&env, tr_k, "s", &key);
                xmlrpc_abort_on_fault(&env);
                xmlrpc_DECREF(tr_k);

                /* Read the values */
                if (!strcmp(key, "brootid")) {
                    xmlrpc_decompose_value(&env, tr_v, "i", &descendent->brootid);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "srpms") && xmlrpc_value_type(tr_v) == XMLRPC_TYPE_ARRAY) {
                    descendent->srpms = read_koji_descendent_results(&env, tr_v);
                } else if (!strcmp(key, "rpms")) {
                    descendent->rpms = read_koji_descendent_results(&env, tr_v);
                } else if (!strcmp(key, "logs")) {
                    descendent->logs = read_koji_descendent_results(&env, tr_v);
                }

                xmlrpc_DECREF(tr_v);
                free(key);
            }

            /* save this descendent in the list */
            TAILQ_INSERT_TAIL(task->descendents, descendent, items);

            xmlrpc_DECREF(dresult);
        }
    }

    xmlrpc_DECREF(result);

    /* Cleanup */
    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();

    return task;
}

/*
 * Return a list of all architectures supported by this Koji instance.
 * NOTE: This should not be called until after config file initialization
 * because we need the Koji settings from the conf file in order to make
 * the XML-RPC calls.
 */
string_list_t *get_all_arches(const struct rpminspect *ri)
{
    string_list_t *arches = NULL;
    string_entry_t *arch = NULL;
    int size = 0;
    int i = 0;
    xmlrpc_env env;
    xmlrpc_server_info *server = NULL;
    xmlrpc_value *fake_params = NULL;
    xmlrpc_value *result = NULL;
    xmlrpc_value *value = NULL;
    char *element = NULL;
    bool have_noarch = false;
    bool have_src = false;

    assert(ri != NULL);

    /* if there is no koji system specified in the configuration, stop */
    if (ri->kojihub == NULL) {
        return NULL;
    }

    /* initialize our list of architectures, always allow 'src' */
    arches = calloc(1, sizeof(*(arches)));
    assert(arches != NULL);
    TAILQ_INIT(arches);

    arch = calloc(1, sizeof(*arch));
    assert(arch != NULL);

    arch->data = strdup(SRPM_ARCH_NAME);
    assert(arch->data != NULL);

    TAILQ_INSERT_TAIL(arches, arch, items);

    /* initialize everything and get XMLRPC ready */
    xmlrpc_env_init(&env);
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, SOFTWARE_NAME, PACKAGE_VERSION, NULL, 0);
    xmlrpc_abort_on_fault(&env);

    /* increase the message response size */
    xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, INT_MAX);

    /*
     * call 'getAllArches' on the koji hub
     * Why not use xmlrpc_client_call() here?  I'm glad you asked.  That
     * function takes a format string and argument list and builds a
     * parameter array for the call.  There's just one problem.  You can't
     * tell it NULL for the format string for calls that require no
     * parameters.  This is a problem for this call because we need to
     * pass nothing.  Within libxmlrpc there is another call we can use
     * but we do still have to create an empty argument array and pass it
     * anyway.  I believe it's better to use xmlrpc_client_call_server_params
     * rather than the xmlrpc_client_call function.  This is what the command
     * line xmlrpc(1) tool does and it works with basically anything.
     */
    server = xmlrpc_server_info_new(&env, ri->kojihub);
    xmlrpc_abort_on_fault(&env);
    fake_params = xmlrpc_array_new(&env);              /* super empty array */
    result = xmlrpc_client_call_server_params(&env, server, "getAllArches", fake_params);
    xmlrpc_abort_on_fault(&env);

    /* is this a valid return value? */
    if (xmlrpc_value_type(result) != XMLRPC_TYPE_ARRAY) {
        xmlrpc_DECREF(result);
        xmlrpc_env_clean(&env);
        xmlrpc_client_cleanup();
        return NULL;
    }

    /* read the values from the result */
    size = xmlrpc_array_size(&env, result);
    xmlrpc_abort_on_fault(&env);

    for (i = 0; i < size; i++) {
        xmlrpc_array_read_item(&env, result, i, &value);
        xmlrpc_abort_on_fault(&env);

        /* Get the array element as a string */
        xmlrpc_decompose_value(&env, value, "s", &element);
        xmlrpc_abort_on_fault(&env);

        /* Flag what we have */
        if (!strcmp(element, RPM_NOARCH_NAME)) {
            have_noarch = true;
        } else if (!strcmp(element, SRPM_ARCH_NAME)) {
            have_src = true;
        }

        /*
         * If the value of an element is nil, just skip over it.  We can't
         * do anything with nil values, so it might as well just not
         * be present in the output.
         */
        if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
            continue;
        }

        /* add this architecture to the list */
        arch = calloc(1, sizeof(*arch));
        assert(arch != NULL);

        arch->data = strdup(element);
        assert(arch->data != NULL);

        TAILQ_INSERT_TAIL(arches, arch, items);
    }

    /* Always add 'noarch' and 'src' to this list */
    if (!have_noarch) {
        arch = calloc(1, sizeof(*arch));
        assert(arch != NULL);
        arch->data = strdup(RPM_NOARCH_NAME);
        assert(arch->data != NULL);
        TAILQ_INSERT_TAIL(arches, arch, items);
    }

    if (!have_src) {
        arch = calloc(1, sizeof(*arch));
        assert(arch != NULL);
        arch->data = strdup(SRPM_ARCH_NAME);
        assert(arch->data != NULL);
        TAILQ_INSERT_TAIL(arches, arch, items);
    }

    /* Cleanup */
    xmlrpc_DECREF(result);

    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();

    return arches;
}
