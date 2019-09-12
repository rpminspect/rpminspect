#!/bin/sh
#
# Create a koji build given a spec file and source in one
# directory.
#

PATH=/usr/bin

if [ -z "${BUILDDIR}" ]; then
    echo "*** BUILDDIR not set" >&2
    exit 1
fi

if [ ! -d "${BUILDDIR}" ]; then
    echo "*** BUILDDIR (${BUILDDIR}) is not a directory" >&2
    exit 1
fi

INPUT="$1"

