# Only perform unit tests if we have CUnit available
if HAVE_CUNIT
TESTS = $(check_PROGRAMS)

check_PROGRAMS = test-badwords \
                 test-koji \
                 test-tty

CFLAGS += -I$(top_srcdir)/src/librpminspect
LDADD = $(top_builddir)/src/librpminspect/librpminspect.la $(CUNIT_LIBS)

test_badwords_SOURCES = test-badwords.c
test_koji_SOURCES = test-koji.c
test_tty_SOURCES = test-tty.c
endif
