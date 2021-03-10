#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
[ -d "${OSDEPS}" ] || exit 1

# Install 32-bit development files on x86_64 systems
if [ "$(uname -m)" = "x86_64" ]; then
    dnf install -y glibc-devel.i686 glibc.i686 libgcc.i686 glibc-headers-x86
fi

# Work around a bug in meson 0.55.0
MESON_VER="$(meson --version 2>/dev/null)"

if [ -z "${MESON_VER}" ] || [ ! "${MESON_VER}" = "0.55.0" ]; then
    exit 0
fi

PYDIR="$(dirname $(rpm -ql meson | grep 'ninjabackend.py$'))"
[ -d "${PYDIR}" ] || exit 2

cd "${PYDIR}"
[ -f "${OSDEPS}/meson.patch" ] || exit 3
patch -p0 < "${OSDEPS}/meson.patch"

# Update the clamav database
freshclam
