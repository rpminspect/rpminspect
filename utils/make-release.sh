#!/bin/sh
#
# Automate making a new release of rpminspect
# This script will increment the version minor number and continue
# through with tagging the repo and make the archive and spec file.
#

PATH=/usr/bin
CWD="$(pwd)"

if [ ! -f "${CWD}/configure.ac" ] && [ ! -f "${CWD}/rpminspect.spec.in" ]; then
    echo "*** You must run this script from the top level rpminspect directory" >&2
    exit 1
fi

git clean -d -x -f
git pull
./autogen.sh
./configure

make bumpver
git add configure.ac
git commit -m "New release"

git clean -d -x -f
./autogen.sh
./configure
make release
make rpminspect.spec
