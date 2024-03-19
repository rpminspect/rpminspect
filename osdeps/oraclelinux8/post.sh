#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# Install 32-bit development files on x86_64 systems
if [ "$(uname -m)" = "x86_64" ]; then
    dnf install -y glibc-devel.i686 glibc.i686 libgcc.i686
fi

# cdson is not [yet] in Oracle Linux
git clone https://github.com/frozencemetery/cdson.git
cd cdson || exit 1
TAG="$(git tag -l | sort -n | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
meson setup build -D prefix=/usr
ninja -C build -v
ninja -C build test
ninja -C build install
cd "${CWD}" || exit 1
rm -rf cdson

# Install libtoml from git
cd "${CWD}" || exit 1
git clone -q https://github.com/ajwans/libtoml.git
cd libtoml || exit 1
cmake .
make
make test
install -D -m 0755 libtoml.so /usr/local/lib/libtoml.so
install -D -m 0644 toml.h /usr/local/include/toml.h
cd "${CWD}" || exit 1
rm -rf libtoml

# Update clamav database
freshclam
