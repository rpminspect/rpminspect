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

#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <CUnit/Basic.h>
#include "rpminspect.h"
#include "test-main.h"

int init_test_inspect_elf(void) {
    init_elf_data();

    if (elf_version(EV_CURRENT) == EV_NONE) {
        return -1;
    }

    return 0;
}

int clean_test_inspect_elf(void) {
    free_elf_data();
    return 0;
}

void test_is_execstack_present(void) {
    int fd;
    Elf *elf;

    /* expected true */
    fd = open("elftest-execstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_TRUE(is_execstack_present(elf));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    /* expected false */
    fd = open("elftest-noexecstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_TRUE(is_execstack_present(elf));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    return;
}

void test_get_execstack_flags(void) {
    int fd;
    Elf *elf;

    /* expected non-zero return */
    fd = open("elftest-execstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_EQUAL(get_execstack_flags(elf), PF_X|PF_W|PF_R);

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    /* expected zero return */
    fd = open("elftest-noexecstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_EQUAL(get_execstack_flags(elf), PF_W|PF_R);

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    return;
}

void test_has_executable_program(void) {
    int fd;
    Elf *elf;

    /* expected true */
    fd = open("elftest-execstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_TRUE(has_executable_program(elf));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    return;
}

void test_is_execstack_valid(void) {
    /* XXX */
    return;
}

void test_is_stack_executable(void) {
    /* XXX */
    return;
}

void test_has_textrel(void) {
    /* XXX */
    return;
}

void test_has_relro(void) {
    /* XXX */
    return;
}

void test_has_bind_now(void) {
    /* XXX */
    return;
}

void test_is_fortified(void) {
    /* XXX */
    return;
}

void test_is_fortifiable(void) {
    /* XXX */
    return;
}

void test_get_fortified_symbols(void) {
    /* XXX */
    return;
}

void test_get_fortifiable_symbols(void) {
    /* XXX */
    return;
}

void test_is_pic_ok(void) {
    /* XXX */
    return;
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("inspect_elf", init_test_inspect_elf, clean_test_inspect_elf);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test is_execstack_present()", test_is_execstack_present) == NULL ||
        CU_add_test(pSuite, "test get_execstack_flags()", test_get_execstack_flags) == NULL ||
        CU_add_test(pSuite, "test has_executable_program()", test_has_executable_program) == NULL ||
        CU_add_test(pSuite, "test is_execstack_valid()", test_is_execstack_valid) == NULL ||
        CU_add_test(pSuite, "test is_stack_executable()", test_is_stack_executable) == NULL ||
        CU_add_test(pSuite, "test has_textrel()", test_has_textrel) == NULL ||
        CU_add_test(pSuite, "test has_relro()", test_has_relro) == NULL ||
        CU_add_test(pSuite, "test has_bind_now()", test_has_bind_now) == NULL ||
        CU_add_test(pSuite, "test is_fortified()", test_is_fortified) == NULL ||
        CU_add_test(pSuite, "test is_fortifiable()", test_is_fortifiable) == NULL ||
        CU_add_test(pSuite, "test get_fortified_symbols()", test_get_fortified_symbols) == NULL ||
        CU_add_test(pSuite, "test get_fortifiable_symbols()", test_get_fortifiable_symbols) == NULL ||
        CU_add_test(pSuite, "test is_pic_ok()", test_is_pic_ok) == NULL) {
        return NULL;
    }

    return pSuite;
}
