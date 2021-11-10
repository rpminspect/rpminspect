#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# There mandoc package in Alpine Linux lacks the library
curl -O https://mandoc.bsd.lv/snapshots/mandoc.tar.gz
SUBDIR="$(tar -tvf mandoc.tar.gz | head -n 1 | rev | cut -d ' ' -f 1 | rev)"
tar -xvf mandoc.tar.gz
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

# unusual workarounds for executable on Alpine Linux
sed -i -e 's|@echo|@/bin/echo|g' "${SUBDIR}"/configure
sed -i -e 's|^int dummy;$|extern int dummy;|g' "${SUBDIR}"/compat_getline.c
sed -i -e 's|^int dummy;$|extern int dummy;|g' "${SUBDIR}"/compat_err.c

( cd "${SUBDIR}" && ./configure && make && make lib-install )
rm -rf mandoc.tar.gz "${SUBDIR}"

# diffstat is not available as a package, so build it
curl -O ftp://ftp.invisible-island.net/diffstat/diffstat.tar.gz
SUBDIR="$(tar -tvf diffstat.tar.gz | head -n 1 | rev | cut -d ' ' -f 1 | rev)"
tar -xvf diffstat.tar.gz
cd "${SUBDIR}" || exit 1
./configure --prefix=/usr/local
make
make install
cd "${CWD}" || exit 1
rm -rf diffstat.tar.gz "${SUBDIR}"

# The 'rc' shell is not available in Arch Linux, build manually
git clone https://github.com/rakitzis/rc.git
cd rc || exit 1
autoreconf -f -i -v
./configure --prefix=/usr/local
make
make install
cd "${CWD}" || exit 1
rm -rf rc

# AT&T Korn Shell is not packaged for Alpine Linux, so cheat here and
# just use 'mksh' for the ksh syntax tests.
ln -s mksh /bin/ksh

# Install libabigail from git
cd "${CWD}" || exit 1
git clone git://sourceware.org/git/libabigail.git
cd libabigail || exit 1
TAG="$(git tag -l | grep ^libabigail- | grep -v '\.rc' | sort -n | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
autoreconf -f -i -v
if grep -q "<libgen\.h>" tools/abisym.cc >/dev/null 2>&1 ; then
    sed -i -r '/^#include\ <elf\.h>/a #include <libgen.h>' tools/abisym.cc
fi
env LIBS="-lfts" ./configure --prefix=/usr/local
make V=1
make install

# Update the clamav database
freshclam
