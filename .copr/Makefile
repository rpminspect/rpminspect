# Copyright (C) 2019 David Cantrell <david.l.cantrell@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

topdir := $(shell realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)

# Pass BUILDTYPE=release to generate a release SRPM
BUILDTYPE ?= copr

# Spec file and template
SPEC_TEMPLATE := $(shell ls -1 $(topdir)/*.spec.in)
SPEC := $(topdir)/$(shell basename $(SPEC_TEMPLATE) .in)

# Replace placeholders in the spec file template
RPMDATE = $(shell date +'%a %b %d %Y')

# Various things we need to generate a tarball
PKG := $(shell grep ^Name: "$(SPEC_TEMPLATE)" | awk '{ print $$2; }')
VER := $(shell grep 'version :' $(topdir)/meson.build | grep -E "'[0-9]+\.[0-9]+'" | cut -d "'" -f 2)

ifeq ($(BUILDTYPE),copr)
GITDATE = $(shell date +'%Y%m%d%H%M')
GITHASH = $(shell git rev-parse --short HEAD)
TARBALL_BASENAME = $(PKG)-$(VER)-$(GITDATE)git$(GITHASH)
endif

ifeq ($(BUILDTYPE),release)
TARBALL_BASENAME = $(PKG)-$(VER)
endif

srpm:
	$(topdir)/.copr/instpkgs.sh $(SPEC_TEMPLATE)
	sed -e 's|%%VERSION%%|$(VER)|g' < "$(SPEC_TEMPLATE)" > "$(SPEC)"
	sed -i -e 's|%%RPMDATE%%|$(RPMDATE)|g' "$(SPEC)"
	sed -i -e 's|%%TARBALL%%|$(TARBALL_BASENAME).tar.xz|g' "$(SPEC)"
	sed -i -e 's|^%setup.*$$|%setup -q -n $(TARBALL_BASENAME)|g' "$(SPEC)"
ifeq ($(BUILDTYPE),copr)
	sed -i -e '/^Release:/ s/1[^%]*/0.1.$(GITDATE)git$(GITHASH)/' "$(SPEC)"
endif
	git archive \
		--format=tar \
		--output='$(topdir)/$(TARBALL_BASENAME).tar' \
		--prefix='$(TARBALL_BASENAME)/' HEAD $(topdir)
	xz -9f $(topdir)/$(TARBALL_BASENAME).tar
	cd $(topdir) ; $(topdir)/utils/mkrpmchangelog.sh --copr > changelog
	rpmbuild \
		-bs --nodeps \
		--define "_sourcedir $(topdir)" \
		--define "_srcrpmdir $(outdir)" \
		--define "_rpmdir $(outdir)" "$(SPEC)"
