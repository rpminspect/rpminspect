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

#include <unistd.h>
#include <CUnit/Basic.h>
#include "rpminspect.h"

int init_test_tty(void) {
    return 0;
}

int clean_test_tty(void) {
    return 0;
}

void test_tty_width(void) {
    size_t w;
    w = tty_width();
    CU_ASSERT(w > 0);
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;
    CU_pTest pTtyWidthTest = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("tty", init_test_tty, clean_test_tty);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if ((pTtyWidthTest = CU_add_test(pSuite, "test tty_width()", test_tty_width)) == NULL) {
        return NULL;
    }

    /* Only run the tty_width test if we have a tty */
    if (!isatty(STDIN_FILENO)) {
        CU_set_test_active(pTtyWidthTest, CU_FALSE);
    }

    return pSuite;
}
