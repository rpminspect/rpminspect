topdir := $(shell realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)

# Spec file and template
SPEC_TEMPLATE = $(topdir)/rpminspect.spec.in
SPEC = $(topdir)/rpminspect.spec

# Replace placeholders in the spec file template
RPMDATE = $(shell date +'%a %b %d %Y')
GITDATE = $(shell date +'%Y%m%d%H%M')
GITHASH = $(shell git rev-parse --short HEAD)

# Various things we need to generate an SRPM
PKG := $(shell grep ^Name: "$(SPEC_TEMPLATE)" | awk '{ print $$2; }')
VER := $(shell grep 'version :' $(topdir)/meson.build | grep -E "'[0-9]+\.[0-9]+'" | cut -d "'" -f 2)

srpm:
	$(topdir)/.copr/installate-package.sh $(topdir)
	sed -e 's|%%VERSION%%|$(VER)|g' < "$(SPEC_TEMPLATE)" > "$(SPEC)"
	sed -i -e 's|%%RPMDATE%%|$(RPMDATE)|g' "$(SPEC)"
	sed -i -e 's|%%GITDATE%%|$(GITDATE)|g' "$(SPEC)"
	sed -i -e 's|%%GITHASH%%|$(GITHASH)|g' "$(SPEC)"
	sed -i -e 's|%%TARBALL%%|$(PKG)-$(VER)-$(GITDATE)git$(GITHASH).tar.xz|g' "$(SPEC)"
	sed -i -e 's|^%setup.*$$|%setup -q -n $(PKG)-$(VER)-$(GITDATE)git$(GITHASH)|g' "$(SPEC)"
	sed -i -e '/^Release:/ s/1[^%]*/0.1.$(GITDATE)git$(GITHASH)/' "$(SPEC)"
	meson setup build
	ninja -v -C build
	ninja -v -C build dist
	cp -p build/meson-dist/$(PKG)-$(VER).tar.xz $(topdir)/$(PKG)-$(VER)-$(GITDATE)git$(GITHASH).tar.xz
	rpmbuild \
		-bs --nodeps \
		--define "_sourcedir $(topdir)" \
		--define "_srcrpmdir $(outdir)" \
		--define "_rpmdir $(outdir)" "$(SPEC)"
