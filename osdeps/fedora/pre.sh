#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
VERSION_ID=0

if [ -r /etc/os-release ] || [ -L /etc/os-release ]; then
    # shellcheck disable=SC1091
    . /etc/os-release
fi

# clamav-devel on Fedora > 44 requires openssl3-devel, which conflicts
# with openssl-devel from OpenSSL 4.x
if [ "${VERSION_ID}" -gt 44 ] 2>/dev/null; then
    dnf install -y openssl3-devel
else
    dnf install -y openssl-devel
fi
