MESON_BUILD_DIR = build
topdir := $(shell realpath $(dir $(lastword $(MAKEFILE_LIST))))

# ninja may be called something else
NINJA := $(shell which ninja 2>/dev/null)
ifeq ($(NINJA),)
NINJA := $(shell which ninja-build 2>/dev/null)
endif
ifeq ($(NINJA),)
NINJA := $(error "Unable to find a suitable `ninja' command in the PATH")
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
		    RPMINSPECT_YAML=$(topdir)/data/rpminspect.yaml \
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
	yum install -y $(shell grep -iE "(Requires|Suggests):" rpminspect.spec.in | grep -v rpminspect | awk '{ print $$2; }' ORS=' ') 

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
