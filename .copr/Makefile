topdir := $(shell realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)

# Spec file and template
SPEC_TEMPLATE = $(topdir)/rpminspect.spec.in
SPEC = $(topdir)/rpminspect.spec

# Compute a date string suitable for RPM spec file changelogs
RPMDATE := $(shell date +'%a %b %d %Y')
RELDATE := $(shell date +'%s')

# Compute a snapshot release number suitable for the RPM spec file
RELEASE := $(shell grep ^Release: "$(SPEC_TEMPLATE)" | awk '{ print $$2; }')
NEWRELEASE := $(shell echo "$(RELEASE)" | sed -e 's|%{.*$$||g').$(RELDATE)%{?dist}

# Various things we need to generate a tarball
PKG := $(shell grep ^Name: "$(SPEC_TEMPLATE)" | awk '{ print $$2; }')
VER := $(shell grep ^AC_INIT "$(topdir)/configure.ac" | cut -d '[' -f 3 | cut -d ']' -f 1)

srpm:
	$(topdir)/.copr/installate-package.sh
	sed -e 's|%%VERSION%%|$(VER)|g' < "$(SPEC_TEMPLATE)" > "$(SPEC)"
	sed -i -e 's|%%RPMDATE%%|$(RPMDATE)|g' "$(SPEC)"
	sed -i -e "/^Release:/ s|$(RELEASE)|$(NEWRELEASE)|g" "$(SPEC)"
	git archive \
		--format=tar.gz \
		--output='$(topdir)/$(PKG)-$(VER).tar.gz' \
		--prefix='$(PKG)-$(VER)/' HEAD $(topdir)
	rpmbuild \
		-bs --nodeps \
		--define "_sourcedir $(topdir)" \
		--define "_srcrpmdir $(outdir)" \
		--define "_rpmdir $(outdir)" "$(SPEC)"
