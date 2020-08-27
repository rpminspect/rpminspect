#!/bin/sh
# Helper script to test rpminspect builds in copr during build
#
# Copyright (C) 2020 Jim Bair <jbair@redhat.com>
#                    David Cantrell <dcantrell@redhat.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# Arguments:
# $1   Full path to rpminspect command to use
# $2   Full path to rpminspect configuration file to use (optional)
#

PATH=/bin:/usr/bin
CMD="$(basename "${0}")"
RPMINSPECT="${1}"
CFG="${2}"

usage() {
    echo "Usage: ${CMD} [exec] [cfg]"
    echo "Arguments:"
    echo "    exec     Full path to 'rpminspect' executable"
    echo "    cfg      Full path to rpminspect configuration file (optional)"
}

if [ ! -x "${RPMINSPECT}" ]; then
    usage
    exit 1
fi

# trap sig 11
trap failure SEGV
failure() {
    echo "ERROR: Caught Signal 11: Aborting" >&2
    exit 1
}

# Packages to diff to consider this build stable-enough
packages='zsh kernel python3 mutt emacs tmux elfutils'

# Validate programs we need are present
reqs='rpmbuild koji'
for req in $reqs; do
    if ! $req --help > /dev/null 2>&1; then
        echo "ERROR: Required program $req missing." >&2
        exit 1
    fi
done

# Find our build environment (fc32 for example)
dist=$(rpmbuild --eval "%{dist}")
if [ -z "${dist}" ] || [ "${dist}" = '%{dist}' ]; then
    echo "ERROR: Unable to find build environment." >&2
    exit 1
fi

# Run comparison against the latest packages, or fail
for package in ${packages}; do
    builds=$(koji list-builds --package="${package}" --state=COMPLETE | grep "${dist}" | tail -n 2 | cut -d ' ' -f 1 | xargs)
    if [ -z "${builds}" ]; then
        echo "ERROR: Unable to find builds for ${package}" >&2
        exit 1
    fi
    echo "INFO: Comparing ${builds}"
    if [ -z "${CFG}" ]; then
        ${RPMINSPECT} -a x86_64,noarch -v -o "${package}".log "${builds}"
    else
        ${RPMINSPECT} -c "${CFG}" -a x86_64,noarch -v -o "${package}".log "${builds}"
    fi
    [ $? -lt 2 ] || exit 1
    echo "INFO: Comparison complete: ${package}.log"
done

# All done, no errors
exit 0
