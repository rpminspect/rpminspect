# Only perform unit tests if we have CUnit available
if HAVE_CUNIT
TESTS = $(check_PROGRAMS)

check_PROGRAMS = test-badwords \
                 test-koji \
                 test-tty \
                 test-strfuncs \
                 test-free \
                 test-init \
                 test-inspect_elf

CFLAGS += -I$(top_srcdir)/src/librpminspect
LDADD = $(top_builddir)/src/librpminspect/librpminspect.la $(CUNIT_LIBS)

test_badwords_SOURCES = test-badwords.c test-main.c
test_koji_SOURCES = test-koji.c test-main.c
test_tty_SOURCES = test-tty.c test-main.c
test_strfuncs_SOURCES = test-strfuncs.c test-main.c
test_free_SOURCES = test-free.c test-main.c
test_init_SOURCES = test-init.c test-main.c
test_inspect_elf_SOURCES = test-inspect_elf.c test-main.c

# Used by the inspect_elf tests
noinst_PROGRAMS = elftest-execstack elftest-noexecstack

elftest_execstack_SOURCES = elftest.c
elftest_execstack_CFLAGS = -z execstack -z relro
elftest_execstack_LDFLAGS = -no-install

elftest_noexecstack_SOURCES = elftest.c
elftest_noexecstack_CFLAGS = -z noexecstack -z relro
elftest_noexecstack_LDFLAGS = -no-install
else

check-local:
	echo "*** CUnit is required to run the tests"; exit 1

endif
