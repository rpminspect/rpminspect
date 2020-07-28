#!/bin/sh
#
# Determine the OS and echo the short name to stdout.
# Not a lot here to keep the variants short.
#

PATH=/bin:/usr/bin
VERSION_ID=0

if [ -r /etc/os-release ]; then
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
        echo "unknown"
    fi
elif [ -r /etc/redhat-release ] && [ "${ID}" = "rhel" ]; then
    v="$(echo "${VERSION_ID}" | cut -d '.' -f 1)"
    if [ "${v}" = "7" ] || [ "${v}" = "8" ]; then
        echo "${ID}${v}"
    else
        echo "unknown"
    fi
elif [ "${ID}" = "opensuse-leap" ] || [ "${ID}" = "ubuntu" ]; then
    echo "${ID}"
else
    echo "unknown"
fi
