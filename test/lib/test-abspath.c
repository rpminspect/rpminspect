/*
 * Copyright (C) 2021  Red Hat, Inc.
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
#include <string.h>
#include <CUnit/Basic.h>
#include "rpminspect.h"

#include "test-main.h"

int init_test_abspath(void) {
    return 0;
}

int clean_test_abspath(void) {
    return 0;
}

void test_abspath(void) {
    char *r = NULL;

    r = abspath("/../lib/jli");
    RI_ASSERT_PTR_NOT_NULL(r);
    RI_ASSERT_EQUAL(strcmp(r, "/lib/jli"), 0);
    free(r);

    r = abspath("/../lib64/");
    RI_ASSERT_PTR_NOT_NULL(r);
    RI_ASSERT_EQUAL(strcmp(r, "/lib64"), 0);
    free(r);

    r = abspath("/usr/lib/../lib64/../lib");
    RI_ASSERT_PTR_NOT_NULL(r);
    RI_ASSERT_EQUAL(strcmp(r, "/usr/lib"), 0);
    free(r);

    r = abspath("/usr/lib/../lib64/../lib/");
    RI_ASSERT_PTR_NOT_NULL(r);
    RI_ASSERT_EQUAL(strcmp(r, "/usr/lib"), 0);
    free(r);
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("abspath", init_test_abspath, clean_test_abspath);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test abspath()", test_abspath) == NULL) {
        return NULL;
    }

    return pSuite;
}
