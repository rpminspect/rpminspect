topdir := $(shell realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)

# Spec file and template
SPEC_TEMPLATE = $(topdir)/rpminspect.spec.in
SPEC = $(topdir)/rpminspect.spec

# Replace placeholders in the spec file template
RPMDATE = $(shell date +'%a %b %d %Y')
GITDATE = $(shell date +'%Y%m%d%H%M')
GITHASH = $(shell git rev-parse --short HEAD)

# Various things we need to generate a tarball
PKG := $(shell grep ^Name: "$(SPEC_TEMPLATE)" | awk '{ print $$2; }')
VER := $(shell grep ^AC_INIT "$(topdir)/configure.ac" | cut -d '[' -f 3 | cut -d ']' -f 1)

srpm:
	$(topdir)/.copr/installate-package.sh
	sed -e 's|%%VERSION%%|$(VER)|g' < "$(SPEC_TEMPLATE)" > "$(SPEC)"
	sed -i -e 's|%%RPMDATE%%|$(RPMDATE)|g' "$(SPEC)"
	sed -i -e 's|%%GITDATE%%|$(GITDATE)|g' "$(SPEC)"
	sed -i -e 's|%%GITHASH%%|$(GITHASH)|g' "$(SPEC)"
	sed -i -e 's|%%TARBALL%%|$(PKG)-$(VER)-$(GITDATE)git$(GITHASH).tar.gz|g' "$(SPEC)"
	git archive \
		--format=tar.gz \
		--output='$(topdir)/$(PKG)-$(VER)-$(GITDATE)git$(GITHASH).tar.gz' \
		--prefix='$(PKG)-$(VER)-$(GITDATE)git$(GITHASH)/' HEAD $(topdir)
	rpmbuild \
		-bs --nodeps \
		--define "_sourcedir $(topdir)" \
		--define "_srcrpmdir $(outdir)" \
		--define "_rpmdir $(outdir)" "$(SPEC)"
