#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

# Install 32-bit development files on x86_64 systems
if [ "$(uname -m)" = "x86_64" ]; then
    yum install -y glibc-devel.i686 glibc.i686 libgcc.i686
fi

# Install libtoml from git
cd "${CWD}" || exit 1
git clone -q https://github.com/ajwans/libtoml.git
cd libtoml || exit 1
cmake .
make
make test
install -D -m 0755 libtoml.so /usr/lib64/libtoml.so
install -D -m 0644 toml.h /usr/include/toml.h
cd "${CWD}" || exit 1
rm -rf libtoml
ldconfig

# Update clamav database
freshclam
