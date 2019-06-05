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

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <xmlrpc-c/client.h>
#include <xmlrpc-c/client_global.h>
#include "rpminspect.h"

/*
 * Local only functions.
 */
static void xmlrpc_abort_on_fault(xmlrpc_env *env) {
    if (env->fault_occurred) {
        fprintf(stderr, "XML-RPC Fault: %s (%d)\n", env->fault_string, env->fault_code);
        fflush(stderr);
        abort();
    }
}

/*
 * Initialize a koji_rpmlist_t.
 */
koji_rpmlist_t *init_koji_rpmlist(void) {
    koji_rpmlist_t *rpms = NULL;

    rpms = calloc(1, sizeof(*(rpms)));
    assert(rpms != NULL);
    TAILQ_INIT(rpms);
    return rpms;
}

/*
 * Free memory associated with a koji_rpmlist_t.
 */
void free_koji_rpmlist(koji_rpmlist_t *rpms) {
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
void init_koji_build(struct koji_build *build) {
    assert(build != NULL);

    build->package_name = NULL;
    build->epoch = NULL;
    build->name = NULL;
    build->version = NULL;
    build->release = NULL;
    build->nvr = NULL;

    build->source = NULL;

    build->creation_time = NULL;
    build->completion_time = NULL;
    build->package_id = -1;
    build->id = -1;
    build->build_id = -1;
    build->state = -1;
    build->completion_ts = 0;
    build->owner_id = -1;
    build->owner_name = NULL;
    build->start_time = NULL;
    build->creation_event_id = -1;
    build->start_ts = 0;
    build->volume_id = -1;
    build->creation_ts = 0;
    build->task_id = -1;

    build->volume_name = NULL;

    build->rpms = init_koji_rpmlist();

    return;
}

/*
 * Free dynamically allocated memory associated with a struct koji_build.
 */
void free_koji_build(struct koji_build *build) {
    assert(build != NULL);

    free(build->package_name);
    free(build->epoch);
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

    free_koji_rpmlist(build->rpms);

    free(build);

    return;
}

/*
 * Look up a Koji build and return the information in a struct koji_build.
 */
struct koji_build *get_koji_build(struct rpminspect *ri, const char *buildspec) {
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
    char *s = NULL;
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

    /* call 'getBuild' on the koji hub */
    result = xmlrpc_client_call(&env, ri->kojihub, "getBuild", "(s)", buildspec);
    xmlrpc_abort_on_fault(&env);

    /* is this a valid build? */
    if (xmlrpc_value_type(result) == XMLRPC_TYPE_NIL) {
        xmlrpc_DECREF(result);
        xmlrpc_env_clean(&env);
        xmlrpc_client_cleanup();
        return NULL;
    }

    /* read the values from the result */
    size = xmlrpc_struct_size(&env, result);
    xmlrpc_abort_on_fault(&env);

    for (i = 0; i < size; i++) {
        xmlrpc_struct_get_key_and_value(&env, result, i, &k, &value);
        xmlrpc_abort_on_fault(&env);

        /* Get the key as a string */
        xmlrpc_parse_value(&env, k, "s", &key);
        xmlrpc_abort_on_fault(&env);

        /*
         * If the value of a key is nil, just skip over it.  We can't
         * do anything with nil values, so it might as well just not
         * be present in the output.
         */
        if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
            continue;
        }

        /*
         * Walk through the keys and fill in the struct.
         * This is tedious, but I would rather do it now rather
         * than unpacking XMLRPC results in later functions.
         */
        if (!strcmp(key, "package_name") && (xmlrpc_value_type(value) == XMLRPC_TYPE_STRING)) {
            xmlrpc_parse_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->package_name = strdup(s);
        } else if (!strcmp(key, "epoch")) {
            xmlrpc_parse_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->epoch = strdup(s);
        } else if (!strcmp(key, "name")) {
            xmlrpc_parse_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->name = strdup(s);
        } else if (!strcmp(key, "version")) {
            xmlrpc_parse_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->version = strdup(s);
        } else if (!strcmp(key, "release")) {
            xmlrpc_parse_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->release = strdup(s);
        } else if (!strcmp(key, "nvr")) {
            xmlrpc_parse_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->nvr = strdup(s);
        } else if (!strcmp(key, "source")) {
            xmlrpc_parse_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->source = strdup(s);
        } else if (!strcmp(key, "creation_time")) {
            xmlrpc_parse_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->creation_time = strdup(s);
        } else if (!strcmp(key, "completion_time")) {
            xmlrpc_parse_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->completion_time = strdup(s);
        } else if (!strcmp(key, "package_id")) {
            xmlrpc_parse_value(&env, value, "i", &build->package_id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "id")) {
            xmlrpc_parse_value(&env, value, "i", &build->id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "build_id")) {
            xmlrpc_parse_value(&env, value, "i", &build->build_id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "state")) {
            xmlrpc_parse_value(&env, value, "i", &build->state);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "completion_ts")) {
            xmlrpc_parse_value(&env, value, "d", &build->completion_ts);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "owner_id")) {
            xmlrpc_parse_value(&env, value, "i", &build->owner_id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "owner_name")) {
            xmlrpc_parse_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->owner_name = strdup(s);
        } else if (!strcmp(key, "start_time")) {
            xmlrpc_parse_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->start_time = strdup(s);
        } else if (!strcmp(key, "creation_event_id")) {
            xmlrpc_parse_value(&env, value, "i", &build->creation_event_id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "start_ts")) {
            xmlrpc_parse_value(&env, value, "d", &build->start_ts);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "volume_id")) {
            xmlrpc_parse_value(&env, value, "i", &build->volume_id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "creation_ts")) {
            xmlrpc_parse_value(&env, value, "d", &build->creation_ts);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "task_id")) {
            xmlrpc_parse_value(&env, value, "i", &build->task_id);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "volume_name")) {
            xmlrpc_parse_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->volume_name = strdup(s);
        }

        xmlrpc_DECREF(k);
        xmlrpc_DECREF(value);
    }

    /* call 'listRPMs' on the koji hub */
    result = xmlrpc_client_call(&env, ri->kojihub, "listBuildRPMs", "(i)", build->build_id);
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
            xmlrpc_struct_get_key_and_value(&env, element, j, &k, &value);
            xmlrpc_abort_on_fault(&env);

            /* Get the key as a string */
            xmlrpc_parse_value(&env, k, "s", &key);
            xmlrpc_abort_on_fault(&env);

            /* Skip nil values */
            if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
                continue;
            }

            /* Grab the values we need */
            if (!strcmp(key, "arch")) {
                xmlrpc_parse_value(&env, value, "s", &s);
                xmlrpc_abort_on_fault(&env);
                rpm->arch = strdup(s);
            } else if (!strcmp(key, "name")) {
                xmlrpc_parse_value(&env, value, "s", &s);
                xmlrpc_abort_on_fault(&env);
                rpm->name = strdup(s);
            } else if (!strcmp(key, "version")) {
                xmlrpc_parse_value(&env, value, "s", &s);
                xmlrpc_abort_on_fault(&env);
                rpm->version = strdup(s);
            } else if (!strcmp(key, "release")) {
                xmlrpc_parse_value(&env, value, "s", &s);
                xmlrpc_abort_on_fault(&env);
                rpm->release = strdup(s);
            } else if (!strcmp(key, "epoch")) {
                xmlrpc_parse_value(&env, value, "i", &rpm->epoch);
            } else if (!strcmp(key, "size")) {
                xmlrpc_parse_value(&env, value, "i", &rpm->size);
            }

            xmlrpc_DECREF(k);
            xmlrpc_DECREF(value);

            if (rpm->arch != NULL && rpm->name != NULL && rpm->version != NULL && rpm->release != NULL) {
                break;
            }
        }

        /* add this rpm to the list */
        TAILQ_INSERT_TAIL(build->rpms, rpm, items);
    }

    /* Cleanup */
    xmlrpc_DECREF(result);

    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();

    return build;
}
