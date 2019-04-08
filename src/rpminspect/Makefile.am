bin_PROGRAMS = rpminspect
dist_man1_MANS = rpminspect.1

confdir = $(sysconfdir)/rpminspect
dist_conf_DATA = rpminspect.conf

EXTRA_DIST = builds.h

rpminspect_SOURCES = rpminspect.c builds.c
rpminspect_CFLAGS = -I$(top_srcdir)/src/librpminspect
rpminspect_LDADD = $(top_builddir)/src/librpminspect/librpminspect.la \
                   $(LIBCURL_LIBS)
