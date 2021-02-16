#!/bin/sh
#
# Determine the OS and echo the short name to stdout.
# Not a lot here to keep the variants short.
#

PATH=/bin:/usr/bin
VERSION_ID=0

if [ -r /etc/os-release ]; then
    # shellcheck disable=SC1091
    . /etc/os-release
fi

if [ -r /etc/fedora-release ] && [ "${ID}" = "fedora" ]; then
    if grep -q -i rawhide /etc/fedora-release >/dev/null 2>&1 ; then
        echo "${ID}-rawhide"
    else
        echo "${ID}"
    fi
elif [ -r /etc/centos-release ] && [ "${ID}" = "centos" ]; then
    if [ ${VERSION_ID} -eq 7 ] || [ ${VERSION_ID} -eq 8 ]; then
        echo "${ID}${VERSION_ID}"
    else
        echo "unknown OS: ${ID}" >&2
    fi
elif [ -r /etc/redhat-release ] && [ "${ID}" = "rhel" ]; then
    v="$(echo "${VERSION_ID}" | cut -d '.' -f 1)"
    if [ "${v}" = "7" ] || [ "${v}" = "8" ]; then
        echo "${ID}${v}"
    else
        echo "unknown OS: ${ID}" >&2
    fi
elif [ "${ID}" = "opensuse-leap" ] || [ "${ID}" = "opensuse-tumbleweed" ]; then
    echo "${ID}"
elif [ "${ID}" = "ubuntu" ] || [ "${ID}" = "debian" ]; then
    echo "${ID}"
elif [ "${ID}" = "slackware" ]; then
    echo "${ID}"
elif [ "${ID}" = "arch" ]; then
    echo "${ID}"
else
    echo "unknown OS: ${ID}" >&2
fi
