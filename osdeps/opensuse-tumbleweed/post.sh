#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# The mandoc package on OpenSUSE lacks libmandoc.a and
# header files, which we need to build rpminspect
curl -O http://mandoc.bsd.lv/snapshots/mandoc.tar.gz
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

# unusual workarounds for executable on OpenSUSE Tumbleweed
sed -i -e 's|@echo|@/bin/echo|g' configure
sed -i -e 's|^int dummy;$|extern int dummy;|g' compat_getline.c
sed -i -e 's|^int dummy;$|extern int dummy;|g' compat_err.c

./configure
make
make lib-install
cd ${CWD}
rm -rf mandoc.tar.gz ${SUBDIR}

# Download 'rc' source from Debian and build it locally
# OpenSUSE Leap lacks the rc shell as an installable package
RC_URL=http://ftp.debian.org/debian/pool/main/r/rc/
ARCHIVE="$(curl -s -L ${RC_URL} | html2text | grep "\.orig\.tar" | cut -d '[' -f 2 | cut -d ']' -f 1 | sort -u | tail -n 1)"
curl -O -L ${RC_URL}/${ARCHIVE}
SUBDIR="$(tar -tvf ${ARCHIVE} | head -n 1 | rev | cut -d ' ' -f 1 | rev)"
cd ${SUBDIR}
./configure --prefix=/usr/local
make
make install
cd ${CWD}
rm -rf ${ARCHIVE} ${SUBDIR}

# Update clamav database
freshclam
