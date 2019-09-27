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

# Spec file and template
SPEC_TEMPLATE := $(shell ls -1 $(topdir)/*.spec.in)
SPEC := $(topdir)/$(shell basename $(SPEC_TEMPLATE) .in)

# Replace placeholders in the spec file template
RPMDATE = $(shell date +'%a %b %d %Y')
GITDATE = $(shell date +'%Y%m%d%H%M')
GITHASH = $(shell git rev-parse --short HEAD)

# Various things we need to generate a tarball
PKG := $(shell grep ^Name: "$(SPEC_TEMPLATE)" | awk '{ print $$2; }')
VER := $(shell grep 'version :' $(topdir)/meson.build | grep -E "'[0-9]+\.[0-9]+'" | cut -d "'" -f 2)

srpm:
	$(topdir)/.copr/instpkgs.sh $(SPEC_TEMPLATE)
	sed -e 's|%%VERSION%%|$(VER)|g' < "$(SPEC_TEMPLATE)" > "$(SPEC)"
	sed -i -e 's|%%RPMDATE%%|$(RPMDATE)|g' "$(SPEC)"
	sed -i -e 's|%%GITDATE%%|$(GITDATE)|g' "$(SPEC)"
	sed -i -e 's|%%GITHASH%%|$(GITHASH)|g' "$(SPEC)"
	sed -i -e 's|%%TARBALL%%|$(PKG)-$(VER)-$(GITDATE)git$(GITHASH).tar.xz|g' "$(SPEC)"
	sed -i -e 's|^%setup.*$$|%setup -q -n $(PKG)-$(VER)-$(GITDATE)git$(GITHASH)|g' "$(SPEC)"
	sed -i -e '/^Release:/ s/1[^%]*/0.1.$(GITDATE)git$(GITHASH)/' "$(SPEC)"
	git archive \
		--format=tar \
		--output='$(topdir)/$(PKG)-$(VER)-$(GITDATE)git$(GITHASH).tar' \
		--prefix='$(PKG)-$(VER)-$(GITDATE)git$(GITHASH)/' HEAD $(topdir)
	xz -9 $(topdir)/$(PKG)-$(VER)-$(GITDATE)git$(GITHASH).tar
	rpmbuild \
		-bs --nodeps \
		--define "_sourcedir $(topdir)" \
		--define "_srcrpmdir $(outdir)" \
		--define "_rpmdir $(outdir)" "$(SPEC)"
