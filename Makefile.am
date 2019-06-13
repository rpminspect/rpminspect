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
	sed -i -e 's|%%TARBALL%%|$(DIST_ARCHIVES)|g' $@

srpm: rpminspect.spec dist
	rpmbuild -bs --nodeps --define "_sourcedir ." --define "_srcrpmdir ." --define "_rpmdir ." rpminspect.spec

# Create a dist archive, tag the repo, and sign the dist archive
release: dist
	git tag -s -a -m "Tag release v$(VERSION)" v$(VERSION)
	gpg --detach-sign --armor $(DIST_ARCHIVES)

clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean
