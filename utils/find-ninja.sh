#!/bin/sh
#
# Find the ninja command to use.
#

if ! ninja --help >&- 2>&- ; then
    echo "ninja"
    exit 0
fi

if ! ninja-build --help >&- 2>&- ; then
    echo "ninja-build"
    exit 0
fi

exit 1
