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
	$(topdir)/release.sh -A

clean:
	-rm -rf $(MESON_BUILD_DIR)
