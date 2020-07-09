MESON_BUILD_DIR = build
topdir := $(shell realpath $(dir $(lastword $(MAKEFILE_LIST))))

# ninja may be called something else
NINJA := $(shell which ninja 2>/dev/null)
ifeq ($(NINJA),)
NINJA := $(shell which ninja-build 2>/dev/null)
endif
ifeq ($(NINJA),)
NINJA = $(error "*** unable to find a suitable `ninja' command in the PATH")
endif

# Additional packages required to run the test suite, varies by OS
OS = $(shell $(TOPDIR)/scripts/determine-os.sh)
PKG_CMD = $(error "*** unable to determine host operating system")
REQS = $(shell grep -iE "(Requires|Suggests):" rpminspect.spec.in | grep -v rpminspect | awk '{ print $$2; }' ORS=' ')

ifeq ($(OS),fedora)
PKG_CMD = dnf install -y
PIP_CMD = pip-3 install
REQS += glibc.i686 glibc-devel.i686 CUnit CUnit-devel kernel-devel \
        libgcc.i686 git rpm-build valgrind libffi-devel make \
        sssd-client python3-pip python3-devel python3-rpm \
        kernel-core python3-pyyaml gcovr
PIP_REQS = cpp-coveralls rpmfluff
endif

ifeq ($(OS),centos8)
PKG_CMD = dnf --enablerepo=PowerTools install -y
PIP_CMD = pip-3 install -I
REQS += glibc.i686 glibc-devel.i686 CUnit CUnit-devel kernel-devel \
        libgcc.i686 git rpm-build valgrind libffi-devel make \
        sssd-client python3-pip python3-devel python3-rpm \
        kernel-core python3-pyyaml
PIP_REQS = cpp-coveralls rpmfluff gcovr PyYAML
endif

ifeq ($(OS),centos7)
PKG_CMD = yum install -y
PIP_CMD = pip-3 install
REQS += glibc.i686 glibc-devel.i686 CUnit CUnit-devel kernel-devel \
        libgcc.i686 git rpm-build valgrind libffi-devel make \
        sssd-client python36-pip python36-devel python36-rpm \
        kernel-core
PIP_REQS = cpp-coveralls rpmfluff gcovr PyYAML
endif

ifeq ($(OS),ubuntu)
PKG_CMD = apt-get -y install
PIP_CMD = pip3 install -I
REQS = gcc meson gettext pkg-config libjson-c-dev libxml2-dev libzstd-dev \
       librpm-dev libarchive-dev libelf-dev libkmod-dev libcurl4-openssl-dev \
       libyaml-dev libssl-dev libcap-dev libxmlrpc-core-c3-dev libcunit1-dev \
       ksh rc tcsh zsh valgrind gcovr python3-setuptools python3-pip \
       libffi-dev rpm desktop-file-utils sssd python3-rpm gcc-multilib \
       linux-headers libc-dev:i386
PIP_REQS = cpp-coveralls rpmfluff PyYAML
endif

# Take additional command line argument as a positional parameter for
# the Makefile target
TARGET_ARG = `arg="$(filter-out $@,$(MAKECMDGOALS))" && echo $${arg:-${1}}`

all: setup
	$(NINJA) -C $(MESON_BUILD_DIR) -v

setup:
	meson setup $(MESON_BUILD_DIR)

check: setup
	@test_name="$(call TARGET_ARG,)" ; \
	if [ -z "$${test_name}" ]; then \
		meson test -C $(MESON_BUILD_DIR) -v ; \
	else \
		test_script="test_$${test_name}.py" ; \
		if [ ! -f "$(topdir)/test/$${test_script}" ]; then \
			echo "*** test/$${test_script} does not exist." >&2 ; \
			exit 1 ; \
		fi ; \
		env RPMINSPECT=$(topdir)/build/src/rpminspect \
		    RPMINSPECT_YAML=$(topdir)/data/generic.yaml \
		    RPMINSPECT_TEST_DATA_PATH=$(topdir)/test/data \
		python3 -Bm unittest discover -v $(topdir)/test/ $${test_script} ; \
	fi

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
	$(topdir)/utils/submit-koji-builds.sh $$(ls -1 $(topdir)/*.tar.*) $$(basename $(topdir))

clean:
	-rm -rf $(MESON_BUILD_DIR)

instreqs:
ifeq ($(OS),ubuntu)
	dpkg --add-architecture i386
	apt-get update
endif
	$(PKG_CMD) $(REQS)
	$(PIP_CMD) $(PIP_REQS)

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
	@echo "    make koji"

# Quiet errors about target arguments not being targets
%:
	@true
