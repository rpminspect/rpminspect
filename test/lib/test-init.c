/*
 * Copyright © 2019 Red Hat, Inc.
 * Author(s): David Cantrell <dcantrell@redhat.com>
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

int init_test_init(void) {
    return 0;
}

int clean_test_init(void) {
    return 0;
}

void test_init_rpminspect(void) {
    struct rpminspect *ri = NULL;

    ri = init_rpminspect(ri, NULL, NULL);
    RI_ASSERT_PTR_NOT_NULL(ri);
    free_rpminspect(ri);
    return;
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("init", init_test_init, clean_test_init);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test init_rpminspect()", test_init_rpminspect) == NULL) {
        return NULL;
    }

    return pSuite;
}
