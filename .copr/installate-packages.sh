#!/bin/sh

PATH=/usr/bin
TOPDIR="$1"

[ ${UID} -ne 0 ] && exit 0

# *sigh*
yum --help >/dev/null 2>&1
[ $? -eq 0 ] && INSTALLATOR=yum || INSTALLATOR=dnf

# Test dependencies not listed as BuildRequires
TEST_PKGS="CUnit-devel valgrind gcovr python3-pip rpm-build"

# The actual BuildRequires
BUILD_REQUIRES="$(grep ^BuildRequires: ${TOPDIR}/rpminspect.spec.in | awk '{ print $2; }')"

${INSTALLATOR} install -y git ${TEST_PKGS} ${BUILD_REQUIRES}
pip-3 install rpmfluff
