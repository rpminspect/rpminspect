# Only perform unit tests if we have CUnit available
if HAVE_CUNIT
TESTS = $(check_PROGRAMS)

check_PROGRAMS = test-badwords

test_badwords_SOURCES = test-badwords.c
test_badwords_CFLAGS = -I$(top_srcdir)/src/librpminspect
test_badwords_LDADD = $(top_builddir)/src/librpminspect/librpminspect.la $(CUNIT_LIBS)
endif
