MESON_BUILD_DIR = build
topdir := $(shell realpath $(dir $(lastword $(MAKEFILE_LIST))))

all: setup
	ninja -C $(MESON_BUILD_DIR) -v

setup:
	meson setup $(MESON_BUILD_DIR)

check: setup
	meson test -C $(MESON_BUILD_DIR) -v

update-pot: setup
	find src -type f -name "*.c" > po/POTFILES.new
	find src -type f -name "*.h" >> po/POTFILES.new
	find lib -type f -name "*.c" >> po/POTFILES.new
	find lib -type f -name "*.h" >> po/POTFILES.new
	sort po/POTFILES.new | uniq > po/POTFILES
	rm -f po/POTFILES.new
	ninja -C $(MESON_BUILD_DIR) rpminspect-pot

srpm:
	$(topdir)/utils/srpm.sh

release:
	$(topdir)/utils/release.sh -A

koji: srpm
	$(topdir)/utils/submit-koji-builds.sh $$(ls -1 $(topdir)/*.tar.*) $$(basename $(topdir))

clean:
	-rm -rf $(MESON_BUILD_DIR)

instreqs:
	sudo yum install -y $(shell grep -i requires: rpminspect.spec.in | grep -v rpminspect | awk '{ print $$2; }' ORS=' ')

help:
	@echo "rpminspect helper Makefile"
	@echo "The source tree uses meson(1) for building and testing, but this Makefile"
	@echo "is intended as a simple helper for the common steps."
	@echo
	@echo "    all          Default target, setup tree to build and build"
	@echo "    setup        Run 'meson setup $(MESON_BUILD_DIR)'"
	@echo "    check        Run 'meson test -C $(MESON_BUILD_DIR) -v'"
	@echo "    update-pot   Update po/POTFILES and po/rpminspect.pot"
	@echo "    srpm         Generate an SRPM package of the latest release"
	@echo "    release      Run 'utils/release.sh -A' to make a new release"
	@echo "    koji         Run 'make srpm' then 'utils/submit-koji-builds.sh'"
	@echo "    clean        Run 'rm -rf $(MESON_BUILD_DIR)'"
	@echo "    instreqs     Install required build and runtime packages"
	@echo
	@echo "To build:"
	@echo "    make"
	@echo
	@echo "To run the test suite:"
	@echo "    make check"
	@echo
	@echo "Make a new release on Github:"
	@echo "    make release"
	@echo
	@echo "Generate SRPM of the latest release and do all Koji builds:"
	@echo "    make koji"
