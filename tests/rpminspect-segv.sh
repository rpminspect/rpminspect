#!/bin/sh

# Verify rpminspect doesn't segfault on release-less args

if [ ! -x "${RPMINSPECT}" ]; then
    echo "*** Missing rpminspect executable" >&2
    exit 1
fi

"${RPMINSPECT}" 42 >/dev/null 2>&1
T1=$?

if [ ${T1} -eq 139 ]; then
    echo "*** rpminspect should not segfault" 2>&1
    exit 1
fi
