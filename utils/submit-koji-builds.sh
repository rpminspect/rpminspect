#!/bin/sh
#
# Build new releases in Koji
#
# Copyright (C) 2019-2020 David Cantrell <david.l.cantrell@gmail.com>
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
#     $1    Path to the release tarball to build
#     $2    Path to the detached signature for the release tarball
#     $3    The name of the project in dist-git

PATH=/usr/bin
CWD="$(pwd)"
WRKDIR="$(mktemp -d)"

# The list of tools that may or may not be installed locally but
# that the script needs.  Extend this list if needed.
TOOLS="klist"

# What dist-git interaction tool we are using, e.g. Fedora is 'fedpkg'
VENDORPKG="fedpkg"

cleanup() {
    rm -rf "${WRKDIR}"
}

trap cleanup EXIT

# Verify specific tools are available
for tool in ${TOOLS} ${VENDORPKG} ; do
    ${tool} >/dev/null 2>&1
    if [ $? -eq 127 ]; then
        echo "*** Missing '${tool}', perhaps 'yum install -y /usr/bin/${tool}'" >&2
        exit 1
    fi
done

# Need a tarball
if [ $# -eq 0 ]; then
    echo "*** Missing tarball of new release" >&2
    exit 1
fi

TARBALL="$(realpath "$1")"

if [ ! -f "${TARBALL}" ]; then
    echo "*** $(basename "${TARBALL}") does not exist" >&2
    exit 1
fi

if ! tar tf "${TARBALL}" >/dev/null 2>&1 ; then
    echo "*** $(basename "${TARBALL}") is not a tar archive" >&2
    exit 1
fi

shift

# Need tarball signature
if [ $# -eq 0 ]; then
    echo "*** Missing detached signature of release tarball" >&2
    exit 1
fi

TARBALL_ASC="$(realpath "$1")"

if [ ! -f "${TARBALL_ASC}" ]; then
    echo "*** $(basename "${TARBALL_ASC}") does not exist" >&2
    exit 1
fi

if [ ! "$(file -b --mime-type "${TARBALL_ASC}")" = "application/pgp-signature" ]; then
    echo "*** $(basename "${TARBALL_ASC}") is not a gpg signature" >&2
    exit 1
fi

shift

# Need project name
if [ $# -eq 0 ]; then
    echo "*** Missing project name" >&2
    exit 1
fi

PROJECT="$1"
shift

# Need a krb5 ticket
klist >/dev/null 2>&1
if [ $? -eq 1 ]; then
    echo "*** You lack an active Kerberos ticket" >&2
    exit 1
fi

klist | grep -q "krbtgt/FEDORAPROJECT.ORG@FEDORAPROJECT.ORG" >/dev/null 2>&1
if [ $? -eq 1 ]; then
    echo "*** You need a FEDORAPROJECT.ORG Kerberos ticket" >&2
    exit 1
fi

GIT_USERNAME="$(git config user.name)"
GIT_USEREMAIL="$(git config user.email)"

cd "${CWD}" || exit
"${CWD}"/utils/mkrpmchangelog.sh > "${WRKDIR}"/newchangelog

cd "${WRKDIR}" || exit
${VENDORPKG} co "${PROJECT}"
cd "${PROJECT}" || exit

# Allow the calling environment to override the list of dist-git branches
if [ -z "${BRANCHES}" ]; then
    BRANCHES="$(git branch -r | grep -vE "(HEAD)" | cut -d '/' -f 2 | sort | xargs)"
fi

for branch in ${BRANCHES} ; do
    git clean -d -x -f
    git config user.name "${GIT_USERNAME}"
    git config user.email "${GIT_USEREMAIL}"

    # make sure we are on the right branch
    ${VENDORPKG} switch-branch ${branch}
    git pull

    # add the new source archive
    ${VENDORPKG} new-sources "${TARBALL}"

    # extract any changelog entries that appeared in the spec file
    sed -n '/^%changelog/,/^%include\ \%{SOURCE1}/p' "${PROJECT}".spec | \
        grep -vE '^(%changelog|%include)' | \
        sed -e :a -e '/./,$!d;/^\n*$/{$d;N;};/\n$/ba' > cl
    [ -s cl ] || rm -f cl

    # update the rolling %changelog in dist-git
    if [ -f changelog ]; then
        if [ -f cl ]; then
            echo >> changelog
            cat changelog >> cl
        else
            mv changelog cl
        fi

        cp "${WRKDIR}"/newchangelog changelog
        echo >> changelog
        cat cl >> changelog
        rm -f cl
    else
        cp "${WRKDIR}"/newchangelog changelog
    fi

    # copy in the new spec file
    cat "${CWD}"/"${PROJECT}".spec > "${PROJECT}".spec

    # commit changes
    git add sources changelog "${PROJECT}".spec
    ${VENDORPKG} ci -c -p -s
    git clean -d -x -f

    # build
    ${VENDORPKG} build --nowait
done
