#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# There is no mandoc package in Arch Linux
curl -O http://mandoc.bsd.lv/snapshots/mandoc.tar.gz
SUBDIR="$(tar -tvf mandoc.tar.gz | head -n 1 | rev | cut -d ' ' -f 1 | rev)"
tar -xf mandoc.tar.gz
{ echo 'PREFIX=/usr/local';
  echo 'BINDIR=/usr/local/bin';
  echo 'SBINDIR=/usr/local/sbin';
  echo 'MANDIR=/usr/local/share/man';
  echo 'INCLUDEDIR=/usr/local/include';
  echo 'LIBDIR=/usr/local/lib';
  echo 'LN="ln -sf"';
  echo 'MANM_MANCONF=mandoc.conf';
  echo 'INSTALL_PROGRAM="install -D -m 0755"';
  echo 'INSTALL_LIB="install -D -m 0644"';
  echo 'INSTALL_HDR="install -D -m 0644"';
  echo 'INSTALL_MAN="install -D -m 0644"';
  echo 'INSTALL_DATA="install -D -m 0644"';
  echo 'INSTALL_LIBMANDOC=1';
  echo 'CFLAGS="-g -fPIC"';
} > "${SUBDIR}"/configure.local

# unusual workarounds for executable on Arch Linux
sed -i -e 's|@echo|@/bin/echo|g' "${SUBDIR}"/configure
sed -i -e 's|^int dummy;$|extern int dummy;|g' "${SUBDIR}"/compat_getline.c
sed -i -e 's|^int dummy;$|extern int dummy;|g' "${SUBDIR}"/compat_err.c

( cd "${SUBDIR}" && ./configure && make && make lib-install )
rm -rf mandoc.tar.gz "${SUBDIR}"

# The 'rc' shell is not available in Arch Linux, build manually
git clone -q https://github.com/rakitzis/rc.git
cd rc || exit 1
autoreconf -f -i -v
./configure --prefix=/usr/local
make
make install
cd "${CWD}" || exit 1
rm -rf rc

# Install libabigail from git
cd "${CWD}" || exit 1
git clone -q git://sourceware.org/git/libabigail.git
cd libabigail || exit 1
TAG="$(git tag -l | grep ^libabigail- | grep -v '\.rc' | sort -n | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
autoreconf -f -i -v
./configure --prefix=/usr/local
make
make install

# cdson is not [yet] in Arch
cd "${CWD}" || exit 1
git clone https://github.com/frozencemetery/cdson.git
cd cdson || exit 1
TAG="$(git tag -l | sort -n | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
meson setup build
ninja -C build -v
ninja -C build test
ninja -C build install
cd "${CWD}" || exit 1
rm -rf cdson

# Update the clamav database
freshclam
