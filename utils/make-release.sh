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

if [ ! -f "${CWD}/configure.ac" ] && [ ! -f "${CWD}/rpminspect.spec.in" ]; then
    echo "*** You must run this script from the top level rpminspect directory" >&2
    exit 1
fi

# Start from a clean tree
git clean -d -x -f
git pull
./autogen.sh
./configure

# Increment the version number and commit that change
make bumpver
git add configure.ac
git commit -m "New release"

# Now clean and make a release with the new version number
git clean -d -x -f
./autogen.sh
./configure
make release
make rpminspect.spec

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
