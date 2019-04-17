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

#include <CUnit/Basic.h>
#include "rpminspect.h"

struct koji_build *build = NULL;
koji_rpmlist_t *list = NULL;

int init_test_koji(void) {
    return 0;
}

int clean_test_koji(void) {
    return 0;
}

void test_init_koji_build(void) {
    build = calloc(1, sizeof(*build));
    CU_ASSERT_PTR_NOT_NULL(build);
    init_koji_build(build);
    CU_ASSERT_PTR_NOT_NULL(build);
}

void test_free_koji_build(void) {
    CU_ASSERT_PTR_NOT_NULL(build);
    free_koji_build(build);
    CU_ASSERT_PTR_NULL(build);
}

void test_init_koji_rpmlist(void) {
    list = init_koji_rpmlist();
    CU_ASSERT_PTR_NOT_NULL(list);
}

void test_free_koji_rpmlist(void) {
    CU_ASSERT_PTR_NOT_NULL(list);
    free_koji_rpmlist(list);
    CU_ASSERT_PTR_NULL(list);
}

int main(void) {
    CU_pSuite pSuite = NULL;

    /* initialize this test registry */
    if (CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }

    /* add a suite to the registry */
    pSuite = CU_add_suite("koji", init_test_koji, clean_test_koji);
    if (pSuite == NULL) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test init_koji_build()", test_init_koji_build) == NULL ||
        CU_add_test(pSuite, "test free_koji_build()", test_free_koji_build) == NULL ||
        CU_add_test(pSuite, "test init_koji_rpmlist()", test_init_koji_rpmlist) == NULL ||
        CU_add_test(pSuite, "test free_koji_rpmlist()", test_free_koji_rpmlist) == NULL) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* run all tests using the CUnit basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
