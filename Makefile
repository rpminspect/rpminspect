MESON_BUILD_DIR = build
topdir := $(shell realpath $(dir $(lastword $(MAKEFILE_LIST))))

all: setup
	ninja -C build -v

setup:
	meson setup $(MESON_BUILD_DIR)

check: setup
	meson test -C build -v

srpm:
	$(topdir)/utils/srpm.sh

release:
	$(topdir)/utils/release.sh -A

koji: check srpm
	$(topdir)/utils/submit-koji-builds.sh $$(ls -1 $(topdir)/*.tar.*) $$(basename $(topdir))

clean:
	-rm -rf $(MESON_BUILD_DIR)
