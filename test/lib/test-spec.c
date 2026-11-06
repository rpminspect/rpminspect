/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <CUnit/Basic.h>
#include <rpm/rpmmacro.h>
#include "rpminspect.h"

#include "test-main.h"

struct rpminspect *ri = NULL;
char tmpdir[] = _BUILDDIR_"/test-spec.XXXXXX";

/* the %global here must not show up in clean_spec */
const char *leaking_spec =
    "%global leakme 1\n"
    "Name: leaking\n"
    "Version: 1.0\n"
    "Release: 1\n"
    "Summary: leaking\n"
    "License: MIT\n"
    "%description\n"
    "leaking\n";

const char *clean_spec =
    "Name: clean\n"
    "Version: 1.0\n"
    "Release: 1\n"
    "Summary: %{?leakme:leaked}%{!?leakme:notleaked}\n"
    "License: MIT\n"
    "%description\n"
    "clean\n";

const char *dist_spec =
    "Name: dist\n"
    "Version: 1.0\n"
    "Release: 1%{?dist}\n"
    "Summary: dist\n"
    "License: MIT\n"
    "%description\n"
    "dist\n";

char *write_spec(const char *name, const char *contents) {
    char *path = NULL;
    FILE *fp = NULL;

    path = joindelim(PATH_SEP, tmpdir, name, NULL);
    assert(path != NULL);
    fp = fopen(path, "w");
    assert(fp != NULL);
    fputs(contents, fp);
    fclose(fp);

    return path;
}

bool spec_has_line(string_list_t *lines, const char *needle) {
    string_entry_t *entry = NULL;

    if (lines == NULL) {
        return false;
    }

    TAILQ_FOREACH(entry, lines, items) {
        if (!strcmp(entry->data, needle)) {
            return true;
        }
    }

    return false;
}

int init_test_spec(void) {
    ri = init_rpminspect(ri, NULL, NULL);

    if (ri == NULL || init_librpm(ri) != 0) {
        return -1;
    }

    if (mkdtemp(tmpdir) == NULL) {
        return -1;
    }

    return 0;
}

int clean_test_spec(void) {
    rmtree(tmpdir, true, false);
    free_rpminspect(ri);
    return 0;
}

void test_read_spec_global_does_not_leak(void) {
    char *leaking = write_spec("leaking.spec", leaking_spec);
    char *clean = write_spec("clean.spec", clean_spec);
    string_list_t *lines = NULL;

    lines = read_spec(ri, leaking);
    RI_ASSERT_PTR_NOT_NULL(lines);
    list_free(lines, free);

    lines = read_spec(ri, clean);
    RI_ASSERT_PTR_NOT_NULL(lines);
    RI_ASSERT_TRUE(spec_has_line(lines, "Summary: notleaked"));
    RI_ASSERT_FALSE(spec_has_line(lines, "Summary: leaked"));
    list_free(lines, free);

    free(leaking);
    free(clean);
}

void test_read_spec_uses_dist(void) {
    char *dist = write_spec("dist.spec", dist_spec);
    char *value = NULL;
    string_list_t *lines = NULL;

    RI_ASSERT_EQUAL(rpmDefineMacro(NULL, "dist .el42", 0), 0);

    lines = read_spec(ri, dist);
    RI_ASSERT_PTR_NOT_NULL(lines);
    RI_ASSERT_TRUE(spec_has_line(lines, "Release: 1.el42"));
    list_free(lines, free);

    /* read_spec() drops the %dist the caller defined */
    value = rpmExpand("%{?dist}", NULL);
    RI_ASSERT_PTR_NOT_NULL(value);
    RI_ASSERT_STRING_NOT_EQUAL(value, ".el42");
    free(value);

    free(dist);
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("spec", init_test_spec, clean_test_spec);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test read_spec() does not leak %global", test_read_spec_global_does_not_leak) == NULL) {
        return NULL;
    }

    if (CU_add_test(pSuite, "test read_spec() uses and drops %dist", test_read_spec_uses_dist) == NULL) {
        return NULL;
    }

    return pSuite;
}
