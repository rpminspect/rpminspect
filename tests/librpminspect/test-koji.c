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

#include "test-main.h"

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
    RI_ASSERT_PTR_NOT_NULL(build);
    init_koji_build(build);
    RI_ASSERT_PTR_NOT_NULL(build);
}

void test_init_koji_rpmlist(void) {
    list = init_koji_rpmlist();
    RI_ASSERT_PTR_NOT_NULL(list);
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("koji", init_test_koji, clean_test_koji);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test init_koji_build()", test_init_koji_build) == NULL ||
        CU_add_test(pSuite, "test init_koji_rpmlist()", test_init_koji_rpmlist) == NULL) {
        return NULL;
    }

    return pSuite;
}
