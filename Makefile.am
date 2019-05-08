include $(top_srcdir)/aminclude_static.am

ACLOCAL_AMFLAGS = -I m4
CLEANFILES = rpminspect.spec *.src.rpm
EXTRA_DIST = rpminspect.spec.in

SUBDIRS = src data tests

# Compute a date string suitable for RPM spec file changelogs
RPMDATE = $(shell date +'%a %b %d %Y')
GITDATE = $(shell date +'%Y%m%d%H%M')
GITHASH = $(shell git rev-parse --short HEAD)

# Generate a spec file and SRPM file
rpminspect.spec: rpminspect.spec.in
	sed -e 's|%%VERSION%%|$(VERSION)|g' < $< > $@
	sed -i -e 's|%%RPMDATE%%|$(RPMDATE)|g' $@
	sed -i -e 's|%%GITDATE%%|$(GITDATE)|g' $@
	sed -i -e 's|%%GITHASH%%|$(GITHASH)|g' $@

srpm: rpminspect.spec dist
	rpmbuild -bs --nodeps --define "_sourcedir ." --define "_srcrpmdir ." --define "_rpmdir ." rpminspect.spec

clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean
