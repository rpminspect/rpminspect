#!/bin/sh
#
# Automate making a new release of rpminspect
# This script will increment the version minor number and continue
# through with tagging the repo and make the archive and spec file.
#

PATH=/usr/bin
CWD="$(pwd)"
CURL="curl -L -O --progress-bar"

# Github settings
PROJECT=rpminspect
GH_API="https://api.github.com"
GH_REPO="${GH_API}/repos/${PROJECT}/${PROJECT}"

if [ ! -f "${CWD}/meson.build" ] && [ ! -f "${CWD}/rpminspect.spec.in" ]; then
    echo "*** You must run this script from the top level rpminspect directory" >&2
    exit 1
fi

# Fail if the git index is unclean
if [ ! -z "$(git status --porcelain)" ]; then
    echo "*** git index is not clean:" >&2
    git status --porcelain | sed -e 's|^|*** |g' >&2
    exit 1
fi

# Increment the version number and commit that change
CURVER="$(grep 'version :' meson.build | grep -E "'[0-9]+\.[0-9]+'" | cut -d "'" -f 2)"
CURMAJ="$(echo ${CURVER} | cut -d '.' -f 1)"
CURMIN="$(echo ${CURVER} | cut -d '.' -f 2)"
NEWMIN="$(expr ${CURMIN} + 1)"
VERSION="${CURMAJ}.${NEWMIN}"
sed -i -e "s|'${CURVER}'|'${VERSION}'|g" meson.build
git add meson.build
git commit -m "New release (${VERSION})"

# Now clean and make a release with the new version number
meson setup build
ninja -v -C build dist
git tag -s -a -m "Tag release v${VERSION}" v${VERSION}
gpg --detach-sign --armor $(DIST_ARCHIVES)
echo
echo "*** $(DIST_ARCHIVES) ready to upload to github"
echo "*** $(DIST_ARCHIVES).asc ready to upload to github"

# Create an rpminspect.spec file
RPMDATE="$(date +'%a %b %d %Y')"
GITDATE="$(date +'%Y%m%d%H%M')"
GITHASH="$(git rev-parse --short HEAD))"
sed -e "s|%%VERSION%%|${VERSION}|g" < rpminspect.spec.in > build/rpminspect.spec
sed -i -e "s|%%RPMDATE%%|${RPMDATE}|g" build/rpminspect.spec
sed -i -e "s|%%GITDATE%%|${GITDATE}|g" build/rpminspect.spec
sed -i -e "s|%%GITHASH%%|${GITHASH}|g" build/rpminspect.spec
sed -i -e "s|%%TARBALL%%|${DIST_ARCHIVES}|g" build/rpminspect.spec

# Generate SRPM
( cd build
  rpmbuild -bs --nodeps \
           --define "_sourcedir ." \
           --define "_srcrpmdir ." \
           --define "_rpmdir ." rpminspect.spec )

# Push the changes
git push
git push --tags
TAG="$(git tag -l | tail -n 1)"
VER="$(echo "${TAG}" | sed -e 's/^v//g')"

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

# XXX: Create new release on github
#API_JSON=$(printf '{"tag_name": "%s", "target_commitish": "master", "name": "%s-%s", "body": "%s-%s", "draft": false, "prerelease": false}' ${TAG} ${PROJECT} ${VER} ${PROJECT} ${VER})
#${CURL} --data "${API_JSON}" https://api.github.com/repos/${PROJECT}/${PROJECT}/releases?access_token=${TOKEN}

# XXX: Get the ID of the asset
#ASSET_ID="$(${CURL} -sH "${GH_AUTH}" "${GH_TAGS}" | grep -m 1 "id.:" | grep -w id | tr : = | tr -cd '[[:alnum:]]=')"
#if [ -z "${ASSET_ID}" ]; then
#    echo "*** Unable to get the asset ID" >&2
#    exit 1
#fi

# XXX: Upload the assets
#for asset in ${PROJECT}-${VER}.tar.gz ${PROJECT}-${VER}.tar.gz.asc ; do
#    GH_ASSET="https://uploads.github.com/repos/${PROJECT}/${PROJECT}/releases/${ASSET_ID}/assets?name=${asset}"
#    ${CURL} "$GITHUB_OAUTH_BASIC" --data-binary @"$filename" -H "${GH_AUTH}" -H "Content-Type: application/octet-stream" ${GH_ASSET}
#done
