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

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>
#include <xmlrpc-c/client.h>
#include <xmlrpc-c/client_global.h>
#include "rpminspect.h"

/*
 * Local only functions.
 */

/*
 * General error handler for xmlrpc failures.  Could be improved
 * a bit to be more helpful to the user.
 */
static void xmlrpc_abort_on_fault(xmlrpc_env *env) {
    if (env->fault_occurred) {
        fprintf(stderr, "XML-RPC Fault: %s (%d)\n", env->fault_string, env->fault_code);
        fflush(stderr);
        abort();
    }
}

/*
 * Initialize a koji_buildlist_t.
 */
koji_buildlist_t *init_koji_buildlist(void) {
    koji_buildlist_t *builds = NULL;

    builds = calloc(1, sizeof(*(builds)));
    assert(builds != NULL);
    TAILQ_INIT(builds);
    return builds;
}

/*
 * Free memory associated with a koji_buildlist_t.
 */
void free_koji_buildlist(koji_buildlist_t *builds) {
    koji_buildlist_entry_t *entry = NULL;

    if (builds == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(builds)) {
        entry = TAILQ_FIRST(builds);
        TAILQ_REMOVE(builds, entry, builditems);

        free(entry->package_name);
        entry->package_name = NULL;

        free_koji_rpmlist(entry->rpms);

        free(entry);
    }

    free(builds);

    return;
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
    build->module_build_service_id = NULL;
    build->module_version = NULL;
    build->module_context = NULL;
    build->module_content_koji_tag = NULL;

    build->builds = init_koji_buildlist();

    return;
}

/*
 * Free dynamically allocated memory associated with a struct koji_build.
 */
void free_koji_build(struct koji_build *build) {
    assert(build != NULL);

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
        xmlrpc_struct_read_member(&env, result, i, &k, &value);
        xmlrpc_abort_on_fault(&env);

        /* Get the key as a string */
        xmlrpc_decompose_value(&env, k, "s", &key);
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
        if (!strcmp(key, "package_name")) {
            xmlrpc_decompose_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->package_name = strdup(s);
        } else if (!strcmp(key, "epoch")) {
            xmlrpc_decompose_value(&env, value, "i", &build->epoch);
            xmlrpc_abort_on_fault(&env);
        } else if (!strcmp(key, "name")) {
            xmlrpc_decompose_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->name = strdup(s);
        } else if (!strcmp(key, "version")) {
            xmlrpc_decompose_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->version = strdup(s);
        } else if (!strcmp(key, "release")) {
            xmlrpc_decompose_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->release = strdup(s);
        } else if (!strcmp(key, "nvr")) {
            xmlrpc_decompose_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->nvr = strdup(s);
        } else if (!strcmp(key, "source")) {
            xmlrpc_decompose_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->source = strdup(s);
        } else if (!strcmp(key, "creation_time")) {
            xmlrpc_decompose_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->creation_time = strdup(s);
        } else if (!strcmp(key, "completion_time")) {
            xmlrpc_decompose_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->completion_time = strdup(s);
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
            xmlrpc_decompose_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->owner_name = strdup(s);
        } else if (!strcmp(key, "start_time")) {
            xmlrpc_decompose_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->start_time = strdup(s);
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
            xmlrpc_decompose_value(&env, value, "s", &s);
            xmlrpc_abort_on_fault(&env);
            build->volume_name = strdup(s);
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

                /* Skip nil values */
                if (xmlrpc_value_type(subv) == XMLRPC_TYPE_NIL) {
                    continue;
                }

                if (!strcmp(subkey, "source") || !strcmp(subkey, "typeinfo") || !strcmp(subkey, "module")) {
                    subsize = xmlrpc_struct_size(&env, subv);
                    xmlrpc_abort_on_fault(&env);
                    value = subv;
                    j = 0;
                    continue;
                } else if (!strcmp(subkey, "original_url")) {
                    xmlrpc_decompose_value(&env, subv, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    build->original_url = strdup(s);
                } else if (!strcmp(subkey, "modulemd_str")) {
                    xmlrpc_decompose_value(&env, subv, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    build->modulemd_str = strdup(s);
                    ri->buildtype = KOJI_BUILD_MODULE;
                } else if(!strcmp(subkey, "name")) {
                    xmlrpc_decompose_value(&env, subv, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    build->module_name = strdup(s);
                } else if(!strcmp(subkey, "stream")) {
                    xmlrpc_decompose_value(&env, subv, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    build->module_stream = strdup(s);
                } else if(!strcmp(subkey, "module_build_service_id")) {
                    xmlrpc_decompose_value(&env, subv, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    build->module_build_service_id = strdup(s);
                } else if(!strcmp(subkey, "version")) {
                    xmlrpc_decompose_value(&env, subv, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    build->module_version = strdup(s);
                } else if(!strcmp(subkey, "context")) {
                    xmlrpc_decompose_value(&env, subv, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    build->module_context = strdup(s);
                } else if(!strcmp(subkey, "content_koji_tag")) {
                    xmlrpc_decompose_value(&env, subv, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    build->module_content_koji_tag = strdup(s);
                }

                j++;
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

                /* Skip nil values */
                if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
                    continue;
                }

                /* Grab the values we need */
                if (!strcmp(key, "build_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->build_id);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "package_name")) {
                    xmlrpc_decompose_value(&env, value, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    buildentry->package_name = strdup(s);
                }
            }

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
                    continue;
                }

                /* Grab the values we need */
                if (!strcmp(key, "arch")) {
                    xmlrpc_decompose_value(&env, value, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    rpm->arch = strdup(s);
                } else if (!strcmp(key, "name")) {
                    xmlrpc_decompose_value(&env, value, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    rpm->name = strdup(s);
                } else if (!strcmp(key, "version")) {
                    xmlrpc_decompose_value(&env, value, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    rpm->version = strdup(s);
                } else if (!strcmp(key, "release")) {
                    xmlrpc_decompose_value(&env, value, "s", &s);
                    xmlrpc_abort_on_fault(&env);
                    rpm->release = strdup(s);
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
 * Return a list of all architectures supported by this Koji instance.
 * NOTE: This should not be called until after config file initialization
 * because we need the Koji settings from the conf file in order to make
 * the XML-RPC calls.
 */
string_list_t *get_all_arches(const struct rpminspect *ri) {
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

    /* Cleanup */
    xmlrpc_DECREF(result);

    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();

    return arches;
}

/*
 * Checks an RPM architecture against the user-specified list.
 * If the user did not specify a list of architectures, return
 * true.  If the user specified a list, only return true if the
 * RPM architecture is in the specified list.  The function
 * returns false if the architecture is not allowed.
 */
bool allowed_arch(const struct rpminspect *ri, const char *rpmarch) {
    string_entry_t *arch = NULL;

    assert(ri != NULL);
    assert(rpmarch != NULL);

    if (ri->arches == NULL) {
        return true;
    }

    TAILQ_FOREACH(arch, ri->arches, items) {
        if (!strcmp(rpmarch, arch->data)) {
            return true;
        }
    }

    return false;
}
