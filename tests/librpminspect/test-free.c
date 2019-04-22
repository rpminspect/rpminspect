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

int init_test_free(void) {
    return 0;
}

int clean_test_free(void) {
    return 0;
}

void test_free_rpminspect(void) {
    struct rpminspect ri;

    CU_ASSERT_EQUAL(init_rpminspect(&ri, NULL), 0);
    CU_ASSERT_PTR_NOT_NULL(ri.cfgfile);
    free_rpminspect(&ri);
    CU_ASSERT_PTR_NULL(ri.workdir);
    return;
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("free", init_test_free, clean_test_free);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test free_rpminspect()", test_free_rpminspect) == NULL) {
        return NULL;
    }

    return pSuite;
}
