#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# There mandoc package in Alpine Linux lacks the library
curl -O https://mandoc.bsd.lv/snapshots/mandoc.tar.gz
SUBDIR="$(tar -tvf mandoc.tar.gz | head -n 1 | rev | cut -d ' ' -f 1 | rev)"
tar -xvf mandoc.tar.gz
cd ${SUBDIR}
echo 'PREFIX=/usr/local'                     > configure.local
echo 'BINDIR=/usr/local/bin'                >> configure.local
echo 'SBINDIR=/usr/local/sbin'              >> configure.local
echo 'MANDIR=/usr/local/share/man'          >> configure.local
echo 'INCLUDEDIR=/usr/local/include'        >> configure.local
echo 'LIBDIR=/usr/local/lib'                >> configure.local
echo 'LN="ln -sf"'                          >> configure.local
echo 'MANM_MANCONF=mandoc.conf'             >> configure.local
echo 'INSTALL_PROGRAM="install -D -m 0755"' >> configure.local
echo 'INSTALL_LIB="install -D -m 0644"'     >> configure.local
echo 'INSTALL_HDR="install -D -m 0644"'     >> configure.local
echo 'INSTALL_MAN="install -D -m 0644"'     >> configure.local
echo 'INSTALL_DATA="install -D -m 0644"'    >> configure.local
echo 'INSTALL_LIBMANDOC=1'                  >> configure.local
echo 'CFLAGS="-g -fPIC"'                    >> configure.local

# unusual workarounds for executable on Alpine Linux
sed -i -e 's|@echo|@/bin/echo|g' configure
sed -i -e 's|^int dummy;$|extern int dummy;|g' compat_getline.c
sed -i -e 's|^int dummy;$|extern int dummy;|g' compat_err.c

./configure
make
make lib-install
cd ${CWD}
rm -rf mandoc.tar.gz ${SUBDIR}

# diffstat is not available as a package, so build it
curl -O ftp://ftp.invisible-island.net/diffstat/diffstat.tar.gz
SUBDIR="$(tar -tvf diffstat.tar.gz | head -n 1 | rev | cut -d ' ' -f 1 | rev)"
tar -xvf diffstat.tar.gz
cd ${SUBDIR}
./configure --prefix=/usr/local
make
make install
cd ${CWD}
rm -rf diffstat.tar.gz ${SUBDIR}

# The 'rc' shell is not available in Arch Linux, build manually
git clone https://github.com/rakitzis/rc.git
cd rc
autoreconf -f -i -v
./configure --prefix=/usr/local
make
make install
cd ${CWD}
rm -rf rc

# AT&T Korn Shell is not packaged for Alpine Linux, so cheat here and
# just use 'mksh' for the ksh syntax tests.
ln -s mksh /bin/ksh

# Install libabigail from git
cd ${CWD}
git clone git://sourceware.org/git/libabigail.git
cd libabigail
TAG="$(git tag -l | grep ^libabigail- | grep -v '\.rc' | sort -n | tail -n 1)"
git checkout -b ${TAG} ${TAG}
autoreconf -f -i -v
grep -q "<libgen\.h>" tools/abisym.cc >/dev/null 2>&1
[ $? -ne 0 ] && sed -i -r '/^#include\ <elf\.h>/a #include <libgen.h>' tools/abisym.cc
env LIBS="-lfts" ./configure --prefix=/usr/local
make V=1
make install

# Update the clamav database
freshclam
