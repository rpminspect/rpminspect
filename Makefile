MESON_BUILD_DIR = build
topdir := $(shell realpath $(dir $(lastword $(MAKEFILE_LIST))))

# Project information (may be an easier way to get this from meson)
PROJECT_NAME = $(shell grep ^project $(topdir)/meson.build | cut -d "'" -f 2)
PROJECT_VERSION = $(shell grep version $(topdir)/meson.build | grep -E ',$$' | cut -d "'" -f 2)

# ninja may be called something else
NINJA := $(shell $(topdir)/utils/find-ninja.sh)
ifeq ($(NINJA),)
NINJA = $(error "*** unable to find a suitable `ninja' command")
endif

# Additional packages required to run the test suite, varies by OS
OS = $(shell $(topdir)/utils/determine-os.sh)

ifeq ($(OS),unknown)
OS = $(error "*** unable to determine host operating system")
endif

-include $(topdir)/osdeps/$(OS)/defs.mk

ifeq ($(PKG_CMD),)
PKG_CMD = $(error "*** unable to determine host operating system")
endif

# Take additional command line argument as a positional parameter for
# the Makefile target
TARGET_ARG = `arg="$(filter-out $@,$(MAKECMDGOALS))" && echo $${arg:-${1}}`

# regexp of email addresses of primary authors on the project
PRIMARY_AUTHORS = dcantrell@redhat.com

# full path to release tarball and detached signature
# (this comes from a 'make release')
RELEASED_TARBALL = $(topdir)/$(MESON_BUILD_DIR)/meson-dist/$(PROJECT_NAME)-$(PROJECT_VERSION).tar.xz
RELEASED_TARBALL_ASC = $(RELEASED_TARBALL).asc

all: setup
	$(NINJA) -C $(MESON_BUILD_DIR) -v

setup:
	meson setup $(MESON_BUILD_DIR)

# NOTE: Set QA_RPATHS=64 so that check-rpaths is disabled during the
# rpmfluff rpmbuild operations.  We want to let bad DT_RPATH values
# through for the test suite so that rpminspect can catch them.  The
# only way to disable check-rpaths in rpmbuild is to use this
# environment variable -or- remove the script(s) from /usr/lib/rpm.
# The environment variable is easier.
check: setup
	@test_name="$(call TARGET_ARG,)" ; \
	if [ -z "$${test_name}" ]; then \
		env QA_RPATHS=64 meson test -C $(MESON_BUILD_DIR) -v ; \
	else \
		test_script="test_$${test_name}.py" ; \
		if [ ! -f "$(topdir)/test/$${test_script}" ]; then \
			echo "*** test/$${test_script} does not exist." >&2 ; \
			exit 1 ; \
		fi ; \
		env RPMINSPECT=$(topdir)/build/src/rpminspect \
		    RPMINSPECT_YAML=$(topdir)/data/generic.yaml \
		    RPMINSPECT_TEST_DATA_PATH=$(topdir)/test/data \
		    QA_RPATHS=64 \
		python3 -Bm unittest discover -v $(topdir)/test/ $${test_script} ; \
	fi

flake8:
	python3 -m flake8

black:
	python3 -m black --check --diff $(topdir)/test/

gate: all
	$(topdir)/utils/gate.sh $(topdir)/build/src/rpminspect

update-pot: setup
	find src -type f -name "*.c" > po/POTFILES.new
	find lib -type f -name "*.c" >> po/POTFILES.new
	find include -type f -name "*.h" >> po/POTFILES.new
	sort -u po/POTFILES.new > po/POTFILES
	rm -f po/POTFILES.new
	$(NINJA) -C $(MESON_BUILD_DIR) rpminspect-pot

srpm:
	$(topdir)/utils/srpm.sh

new-release:
	$(topdir)/utils/release.sh -A

release:
	$(topdir)/utils/release.sh -t -p

koji: srpm
	@if [ ! -f $(RELEASED_TARBALL) ]; then \
		echo "*** Missing $(RELEASED_TARBALL), be sure to have run 'make release'" >&2 ; \
		exit 1 ; \
	fi
	@if [ ! -f $(RELEASED_TARBALL_ASC) ]; then \
		echo "*** Missing $(RELEASED_TARBALL_ASC), be sure to have run 'make release'" >&2 ; \
		exit 1 ; \
	fi
	$(topdir)/utils/submit-koji-builds.sh $(RELEASED_TARBALL) $(RELEASED_TARBALL_ASC) $$(basename $(topdir))

clean:
	-rm -rf $(MESON_BUILD_DIR)

instreqs:
	if [ -x $(topdir)/osdeps/$(OS)/pre.sh ]; then \
		env OSDEPS=$(topdir)/osdeps/$(OS) $(topdir)/osdeps/$(OS)/pre.sh ; \
	fi
	$(PKG_CMD) $$(grep -v ^# $(topdir)/osdeps/$(OS)/reqs.txt 2>/dev/null | awk 'NF' ORS=' ')
	$(PIP_CMD) $$(grep -v ^# $(topdir)/osdeps/$(OS)/pip.txt 2>/dev/null | awk 'NF' ORS=' ')
	if [ -x $(topdir)/osdeps/$(OS)/post.sh ]; then \
		env OSDEPS=$(topdir)/osdeps/$(OS) $(topdir)/osdeps/$(OS)/post.sh ; \
	fi

authors:
	echo "Primary Authors" > AUTHORS.md
	echo "===============" >> AUTHORS.md
	echo >> AUTHORS.md
	git log --pretty="%an <%ae>" | sort -u | grep -E "$(PRIMARY_AUTHORS)" | sed -e 's|^|- |g' | sed G >> AUTHORS.md
	echo >> AUTHORS.md
	echo "Contributors" >> AUTHORS.md
	echo "============" >> AUTHORS.md
	echo >> AUTHORS.md
	git log --pretty="%an <%ae>" | sort -u | grep -vE "$(PRIMARY_AUTHORS)" | sed -e 's|^|- |g' | sed G >> AUTHORS.md

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
	@echo "    release      Tag and push current tree as a release"
	@echo "    new-release  Bump version, tag, and push current tree as arelease"
	@echo "    koji         Run 'make srpm' then 'utils/submit-koji-builds.sh'"
	@echo "    clean        Run 'rm -rf $(MESON_BUILD_DIR)'"
	@echo "    instreqs     Install required build and runtime packages"
	@echo "    authors      Generate a new AUTHORS.md file"
	@echo
	@echo "To build:"
	@echo "    make"
	@echo
	@echo "To run the test suite:"
	@echo "    make check"
	@echo
	@echo "To run a single test script (e.g., test_elf.py):"
	@echo "    make check elf"
	@echo
	@echo "Make a new release on Github:"
	@echo "    make release         # just tags and pushes"
	@echo "    make new-release     # bumps version number, tags, and pushes"
	@echo
	@echo "Generate SRPM of the latest release and do all Koji builds:"
	@echo "    env BRANCHES=\"master f31 f32 f33 epel7 epel8\" make koji"
	@echo "NOTE: You must set the BRANCHES environment variable for the koji target"
	@echo "otherwise it will just build for the master branch."

# Quiet errors about target arguments not being targets
%:
	@true
