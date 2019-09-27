#!/bin/sh

PATH=/usr/bin
SPEC="$1"

[ ${UID} -ne 0 ] && exit 0
[ -f "${SPEC}" ] || exit 1

# *sigh*
yum --help >/dev/null 2>&1
[ $? -eq 0 ] && TOOL=yum || TOOL=dnf

# The actual BuildRequires
BUILD_REQUIRES="$(grep ^BuildRequires: ${SPEC} | awk '{ print $2; }')"

${TOOL} install -y git ${BUILD_REQUIRES}
