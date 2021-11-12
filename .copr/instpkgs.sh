#!/bin/sh

PATH=/usr/bin
SPEC="$1"

[ "$(id -u)" -ne 0 ] && exit 0
[ -f "${SPEC}" ] || exit 1

# *sigh*
if yum --help >/dev/null 2>&1 ; then
    TOOL=yum
else
    TOOL=dnf
fi

# The actual BuildRequires
BUILD_REQUIRES="$(rpmspec -q --buildrequires "${SPEC}" | xargs)"
${TOOL} install -y git ${BUILD_REQUIRES}
