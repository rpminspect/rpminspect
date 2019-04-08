#!/bin/sh
# 
# Add a datestamp to the package Release number so that
# yum/dnf upgrade works for nightly builds.
#

PATH=/usr/bin
SPEC="${1}"

[ -f "${SPEC}" ] || exit 1

RELEASE="$(grep ^Release: "${SPEC}" | awk '{ print $2; }')"
NEWRELEASE="$(echo "${RELEASE}" | sed -e 's|%{.*$||g').$(date +%s)%{?dist}"
sed -i -e "/^Release:/ s|${RELEASE}|${NEWRELEASE}|g" "${SPEC}"
