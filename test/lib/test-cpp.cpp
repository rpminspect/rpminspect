/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <string>
#include <string.h>
#include <CUnit/Basic.h>
#include "rpminspect.h"

#include "test-main.h"

int init_test_cpp(void)
{
    return 0;
}

int clean_test_cpp(void)
{
    return 0;
}

void test_cpp(void)
{
    std::string msg = "Hello";

    msg += ", world!";
    RI_ASSERT_EQUAL(strcmp(msg.c_str(), "Hello, world!"), 0);

    return;
}

CU_pSuite get_suite(void)
{
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("cpp", init_test_cpp, clean_test_cpp);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test c++ support", test_cpp) == NULL) {
        return NULL;
    }

    return pSuite;
}
