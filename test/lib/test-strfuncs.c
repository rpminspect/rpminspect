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

#define LOREM_IPSUM "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur."

int init_test_strfuncs(void) {
    return 0;
}

int clean_test_strfuncs(void) {
    return 0;
}

void test_strprefix(void) {
    RI_ASSERT_TRUE(strprefix("flargenblarfle", "flarg"));
    RI_ASSERT_FALSE(strprefix("flargenblarfle", "monkey"));
}

void test_strsuffix(void) {
    RI_ASSERT_TRUE(strsuffix("flargenblarfle", "blarfle"));
    RI_ASSERT_FALSE(strsuffix("flargenblarfle", "monkey"));
}

void test_printwrap(void) {
    FILE *output = fopen("/dev/null", "w+");

    RI_ASSERT_PTR_NOT_NULL(output);
    RI_ASSERT_EQUAL(printwrap(LOREM_IPSUM, 40, 0, output), 8);
    RI_ASSERT_EQUAL(fclose(output), 0);
}

void test_strseverity(void) {
    RI_ASSERT_EQUAL(strcmp(strseverity(RESULT_OK), "OK"), 0);
    RI_ASSERT_EQUAL(strcmp(strseverity(RESULT_INFO), "INFO"), 0);
    RI_ASSERT_EQUAL(strcmp(strseverity(RESULT_WAIVED), "WAIVED"), 0);
    RI_ASSERT_EQUAL(strcmp(strseverity(RESULT_VERIFY), "VERIFY"), 0);
    RI_ASSERT_EQUAL(strcmp(strseverity(RESULT_BAD), "BAD"), 0);
    RI_ASSERT_EQUAL(strcmp(strseverity(-1), "UnKnOwN"), 0);
}

void test_strwaiverauth(void) {
    RI_ASSERT_EQUAL(strcmp(strwaiverauth(NOT_WAIVABLE), "Not Waivable"), 0);
    RI_ASSERT_EQUAL(strcmp(strwaiverauth(WAIVABLE_BY_ANYONE), "Anyone"), 0);
    RI_ASSERT_EQUAL(strcmp(strwaiverauth(WAIVABLE_BY_SECURITY), "Security"), 0);
    RI_ASSERT_EQUAL(strcmp(strwaiverauth(-1), "UnKnOwN"), 0);
}

void test_strreplace(void) {
    ASSERT_AND_FREE(strreplace("", "", ""), "");
    ASSERT_AND_FREE(strreplace("start match", "start", "replace"), "replace match");
    ASSERT_AND_FREE(strreplace("match end", "end", "replace"), "match replace");
    ASSERT_AND_FREE(strreplace("match middle of string", "middle", "replace"), "match replace of string");
    ASSERT_AND_FREE(strreplace("no match", "nothing", "replace"), "no match");
    ASSERT_AND_FREE(strreplace("match several substrings in several places", "a", "replace"),
            "mreplacetch severreplacel substrings in severreplacel plreplaceces");
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("strfuncs", init_test_strfuncs, clean_test_strfuncs);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test strprefix()", test_strprefix) == NULL ||
        CU_add_test(pSuite, "test strsuffix()", test_strsuffix) == NULL ||
        CU_add_test(pSuite, "test printwrap()", test_printwrap) == NULL ||
        CU_add_test(pSuite, "test strseverity()", test_strseverity) == NULL ||
        CU_add_test(pSuite, "test strwaiverauth()", test_strwaiverauth) == NULL ||
        CU_add_test(pSuite, "test strreplace()", test_strreplace) == NULL) {
        return NULL;
    }

    return pSuite;
}
