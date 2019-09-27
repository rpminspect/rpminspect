#!/bin/sh
#
# Automate making a new release of meson-built projects that also
# use Copr for automated builds and that carry template RPM spec
# files.  See README for more information.
#
# Copyright (C) 2019 David Cantrell <david.l.cantrell@gmail.com>
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

PATH=/usr/bin
CWD="$(pwd)"
CURL="curl -L --progress-bar"
PROG="$(basename $0)"

# Github settings
PROJECT="$(basename $(realpath ${CWD}))"
GH_API="https://api.github.com"
GH_REPO="${GH_API}/repos/${PROJECT}/${PROJECT}"

# Command line options
OPT_BUMPVER=
OPT_TAG=
OPT_PUSH=
OPT_GITHUB=

usage() {
    echo "Create a release for ${PROJECT}"
    echo "${PROG} [options]"
    echo "Options:"
    echo "    -b, --bumpver      Increment the minor version number"
    echo "    -t, --tag          Tag the release in git"
    echo "    -p, --push         Push the release and release tag"
    echo "    -g, --github       Create github release entry and upload"
    echo "                       artifacts"
    echo "    -A, --all          Same as '-b -t -p -g'"
    echo "    -h, --help         Display usage"
    echo "The options control behavior on the repo and upstream."
    echo
    echo "The bumpver option increments the minor version number and commits"
    echo "that to the repo (e.g., version 0.5 becomes 0.6). Only the"
    echo "development team should explicitly increment the major version"
    echo "number."
    echo
    echo "The tag option will tag the released version in git locally. So,"
    echo "the 0.6 release becomes tag v0.6 in the local git repo."
    echo
    echo "The push option pushes the version increment commit and release"
    echo "tag to the upstream git repo."
    echo
    echo "The github option generates a new github release entry and uploads"
    echo "the dist artifacts there."
}

# Handle command line options
OPTS=$(getopt -o 'btpgAh' --long 'bumpver,tag,push,github,all,help' -n "${PROG}" -- "$@")

if [ $? -ne 0 ]; then
    echo "Terminating..." >&2
    exit 1
fi

eval set -- "${OPTS}"
unset OPTS

while true ; do
    case "$1" in
        '-b'|'--bumpver')
            OPT_BUMPVER=y
            shift
            ;;
        '-t'|'--tag')
            OPT_TAG=y
            shift
            ;;
        '-p'|'--push')
            OPT_PUSH=y
            shift
            ;;
        '-g'|'--github')
            OPT_GITHUB=y
            shift
            ;;
        '-A'|'--all')
            OPT_BUMPVER=y
            OPT_TAG=y
            OPT_PUSH=y
            OPT_GITHUB=y
            shift
            ;;
        '-h'|'--help')
            usage
            exit 0
            ;;
        '--')
            shift
            break
            ;;
        *)
            echo "Internal error" >&2
            exit 1
            ;;
    esac
done

# This must be a git project
if [ ! -d ${CWD}/.git ] && [ ! -f ${CWD}/.git/config ]; then
    echo "*** Missing .git subdirectory." >&2
    exit 1
fi

# Make sure we are in the right place
if [ ! -f "${CWD}/meson.build" ] && [ ! -f "${CWD}/${PROJECT}.spec.in" ]; then
    echo "*** You must run this script from the top level ${PROJECT} directory" >&2
    exit 1
fi

# Fail if the git index is unclean
if [ ! -z "$(git status --porcelain)" ]; then
    echo "*** git index is not clean:" >&2
    git status --porcelain | sed -e 's|^|*** |g' >&2
    exit 1
fi

# Collect the Github project owner (not necessarily the same as project name)
OWNER="$(grep "git@github.com" .git/config | cut -d ':' -f 2 | cut -d '/' -f 1)"
if [ -z "${OWNER}" ]; then
    echo "*** Unable to determine Github project owner." >&2
    exit 1
fi

# Increment the version number and commit that change
VERSION="$(grep 'version :' meson.build | grep -E "'[0-9]+\.[0-9]+'" | cut -d "'" -f 2)"
CURMAJ="$(echo ${VERSION} | cut -d '.' -f 1)"
CURMIN="$(echo ${VERSION} | cut -d '.' -f 2)"

if [ "${OPT_BUMPVER}" = "y" ]; then
    OLDVERSION="${VERSION}"
    NEWMIN="$(expr ${CURMIN} + 1)"
    VERSION="${CURMAJ}.${NEWMIN}"
    sed -i -e "s|'${OLDVERSION}'|'${VERSION}'|g" meson.build
    git add meson.build
    git commit -m "New release (${VERSION})"
else
    OLDVERSION="${VERSION}"
fi



# Now tag the release
if [ "${OPT_TAG}" = "y" ]; then
    git tag -s -a -m "Tag release v${VERSION}" v${VERSION}
fi

# Generate the dist artifact and sign it
meson setup build
ninja -v -C build dist
cd build/meson-dist
gpg --detach-sign --armor ${PROJECT}-${VERSION}.tar.xz
cd ${CWD}

# Create a spec file
RPMDATE="$(date +'%a %b %d %Y')"
GITDATE="$(date +'%Y%m%d%H%M')"
GITHASH="$(git rev-parse --short HEAD))"
sed -e "s|%%VERSION%%|${VERSION}|g" < ${PROJECT}.spec.in > build/meson-dist/${PROJECT}.spec
sed -i -e "s|%%RPMDATE%%|${RPMDATE}|g" build/meson-dist/${PROJECT}.spec
sed -i -e "s|%%GITDATE%%|${GITDATE}|g" build/meson-dist/${PROJECT}.spec
sed -i -e "s|%%GITHASH%%|${GITHASH}|g" build/meson-dist/${PROJECT}.spec
sed -i -e "s|%%TARBALL%%|${PROJECT}-${VERSION}.tar.xz|g" build/meson-dist/${PROJECT}.spec

# Generate SRPM
( cd build/meson-dist
  rpmbuild -bs --nodeps \
           --define "_sourcedir ." \
           --define "_srcrpmdir ." \
           --define "_rpmdir ." ${PROJECT}.spec )

# Push the changes
if [ "${OPT_PUSH}" = "y" ]; then
    git push
    git push --tags
fi

# Create a github release entry
if [ "${OPT_GITHUB}" = "y" ]; then
    TAG="$(git tag -l | tail -n 1)"
    OLDTAG="$(git tag -l | tail -n 2 | head -n 1)"

    # Get github access token
    TOKEN="$(cat ${HOME}/.githubtoken 2>/dev/null)"
    if [ -z "${TOKEN}" ]; then
        echo "*** Missing github access token from ${HOME}/.githubtoken" >&2
        exit 1
    fi

    # More Github settings
    GH_TAGS="${GH_REPO}/releases/tags/${TAG}"
    GH_AUTH="Authorization: token ${TOKEN}"

    # Validate Github token
    ${CURL} -o /dev/null -sH "${GH_AUTH}" "${GH_REPO}"
    if [ $? -ne 0 ]; then
        echo "*** Invalid Github token, repo, or a network issue" >&2
        exit 1
    fi

    # Create new release on github
    BODY="$(git log --format="%s" ${OLDTAG}.. | sed -e 's|^|* |g')"
    API_JSON="{\"tag_name\": \"${TAG}\", \"target_commitish\": \"master\", \"name\": \"${PROJECT}-${VERSION}\", \"body\": \"${PROJECT}-${VERSION}\", \"draft\": false, \"prerelease\": false}"
    RELEASE_INFO="$(mktemp)"
    ${CURL} -o "${RELEASE_INFO}" --data "${API_JSON}" https://api.github.com/repos/${OWNER}/${PROJECT}/releases?access_token=${TOKEN}

    # Get the ID of the asset
    ASSET_ID="$(grep -m 1 "id.:" ${RELEASE_INFO} | grep -w id | tr : = | tr -cd '[[:alnum:]]=' | cut -d '=' -f 2)"
    rm -f ${RELEASE_INFO}
    if [ -z "${ASSET_ID}" ]; then
        echo "*** Unable to get the asset ID" >&2
        exit 1
    fi

    # Upload the assets
    cd build/meson-dist
    for asset in ${PROJECT}-${VERSION}.tar.xz ${PROJECT}-${VERSION}.tar.xz.asc ; do
        GH_ASSET="https://uploads.github.com/repos/${OWNER}/${PROJECT}/releases/${ASSET_ID}/assets?name=${asset}"
        ${CURL} -H "${GH_AUTH}" --data-binary @"${asset}" -H "Content-Type: application/octet-stream" ${GH_ASSET}
    done
    cd ${CWD}
fi

echo
echo "Log entries since the last release can be obtained with:"
echo "    git log v${OLDVERSION}.."
echo
