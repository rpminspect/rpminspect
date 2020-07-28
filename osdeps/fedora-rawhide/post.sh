#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
[ -d "${OSDEPS}" ] || exit 1

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

