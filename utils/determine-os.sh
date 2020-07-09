#!/bin/sh
#
# Determine the OS and echo the short name to stdout.
# Not a lot here to keep the variants short.
#

PATH=/bin:/usr/bin
CWD="$(pwd)"
VERSION_ID=0

if [ -f /etc/os-release ]; then
    . /etc/os-release
fi

if [ -f /etc/fedora-release ] && [ "${ID}" = "fedora" ]; then
    echo "${ID}"
elif [ -f /etc/centos-release ] && [ "${ID}" = "centos" ]; then
    if [ ${VERSION_ID} -eq 7 ] || [ ${VERSION_ID} -eq 8 ]; then
        echo "${ID}${VERSION_ID}"
    else
        echo "unknown"
    fi
elif [ -f /etc/redhat-release ] && [ "${ID}" = "rhel" ]; then
    v="$(echo "${VERSION_ID}" | cut -d '.' -f 1)"
    if [ "${v}" = "7" ] || [ "${v}" = "8" ]; then
        echo "${ID}${v}"
    else
        echo "unknown"
    fi
elif [ -f /etc/SuSE-release ] && [ "${ID}" = "opensuse-leap" ]; then
    echo "${ID}"
elif [ "${ID}" = "ubuntu" ]; then
    echo "${ID}"
else
    echo "unknown"
fi
