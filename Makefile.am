include $(top_srcdir)/aminclude_static.am

ACLOCAL_AMFLAGS = -I m4
CLEANFILES = rpminspect.spec *.src.rpm
EXTRA_DIST = rpminspect.spec.in meson.build

SUBDIRS = src data tests

# Compute a date string suitable for RPM spec file changelogs
RPMDATE = $(shell date +'%a %b %d %Y')
GITDATE = $(shell date +'%Y%m%d%H%M')
GITHASH = $(shell git rev-parse --short HEAD)

# Current version information
CURVER = $(shell grep AC_INIT configure.ac | cut -d ' ' -f 2 | cut -d '[' -f 2 | cut -d ']' -f 1)
CURMAJ = $(shell echo $(CURVER) | cut -d '.' -f 1)
CURMIN = $(shell echo $(CURVER) | cut -d '.' -f 2)
NEWMIN = $(shell expr $(CURMIN) + 1)

# Generate a spec file and SRPM file
rpminspect.spec: rpminspect.spec.in
	sed -e 's|%%VERSION%%|$(VERSION)|g' < $< > $@
	sed -i -e 's|%%RPMDATE%%|$(RPMDATE)|g' $@
	sed -i -e 's|%%GITDATE%%|$(GITDATE)|g' $@
	sed -i -e 's|%%GITHASH%%|$(GITHASH)|g' $@
	sed -i -e 's|%%TARBALL%%|$(DIST_ARCHIVES)|g' $@

srpm: rpminspect.spec dist
	rpmbuild -bs --nodeps --define "_sourcedir ." --define "_srcrpmdir ." --define "_rpmdir ." rpminspect.spec

# Given an x.y version scheme, increment only the y part.  Assume major and
# minor numbers are integers.
bumpver:
	sed -i -e '/^AC_INIT/ s/\[$(CURVER)\]/[$(CURMAJ).$(NEWMIN)]/' configure.ac

# Create a dist archive, tag the repo, and sign the dist archive
release: dist
	git tag -s -a -m "Tag release v$(VERSION)" v$(VERSION)
	gpg --detach-sign --armor $(DIST_ARCHIVES)
	@echo
	@echo "*** $(DIST_ARCHIVES) ready to upload to github"
	@echo "*** $(DIST_ARCHIVES).asc ready to upload to github"

clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean
