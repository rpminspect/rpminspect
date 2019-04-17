include $(top_srcdir)/aminclude_static.am

bin_PROGRAMS = rpminspect
dist_man1_MANS = rpminspect.1

confdir = $(sysconfdir)/rpminspect
dist_conf_DATA = rpminspect.conf

EXTRA_DIST = builds.h

rpminspect_SOURCES = rpminspect.c builds.c
rpminspect_CPPFLAGS = -I$(top_srcdir)/src/librpminspect $(LIBCURL_CFLAGS) $(CODE_COVERAGE_CPPFLAGS)
rpminspect_CFLAGS = $(CODE_COVERAGE_CFLAGS)
rpminspect_LDADD = $(top_builddir)/src/librpminspect/librpminspect.la \
                   $(LIBCURL_LIBS) $(CODE_COVERAGE_LIBS)

clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean
