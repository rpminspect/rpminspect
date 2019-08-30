#!/bin/sh
# Verify --help gives help output.

if [ ! -x "${RPMINSPECT}" ]; then
    echo "*** Missing rpminspect executable" >&2
    exit 1
fi

output="$(${RPMINSPECT} --help 2>/dev/null)"

if [ $? -ne 0 ]; then
    echo "*** rpminspect exited non-zero from --help" >&2
    exit 1
fi

echo "${output}" | grep -q "^Usage:" >/dev/null 2>&1
T1=$?
echo "${output}" | grep -q "^Options:" >/dev/null 2>&1
T2=$?

if [ ${T1} -ne 0 ] || [ ${T2} -ne 0 ]; then
    echo "*** 'rpminspect --help' did not produce the usage screen" >&2
    exit 1
fi

exit 0
