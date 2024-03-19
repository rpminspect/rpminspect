#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# Install 32-bit development files on x86_64 systems
if [ "$(uname -m)" = "x86_64" ]; then
    yum install -y glibc-devel.i686 glibc.i686 libgcc.i686
fi

# Have to build and install dash manually because it does not exist in
# EL9 as an RPM
git clone https://git.kernel.org/pub/scm/utils/dash/dash.git
cd dash || exit 1
TAG="$(git tag -l | sort -V | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
./autogen.sh
./configure --prefix=/usr/local
make V=1
make V=1 install
cd "${CWD}" || exit 1

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
