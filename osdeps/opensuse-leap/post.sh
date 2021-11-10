#!/bin/sh -x
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# The mandoc package on OpenSUSE lacks libmandoc.a and
# header files, which we need to build rpminspect
curl -O http://mandoc.bsd.lv/snapshots/mandoc.tar.gz
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
( cd "${SUBDIR}" && ./configure && make && make lib-install )
rm -rf mandoc.tar.gz "${SUBDIR}"

# Download 'rc' source from Debian and build it locally
# OpenSUSE Leap lacks the rc shell as an installable package
#
# NOTE: The ARCHIVE= line depends on the output of html2text.  You
# must ensure you are looking at the output on the target system.  The
# default html2text output on Fedora is not the same as what you see
# on OpenSUSE, for instance.
RC_URL=http://ftp.debian.org/debian/pool/main/r/rc
ARCHIVE="$(curl -s -L ${RC_URL} | html2text | grep "\.orig\.tar" | awk '{ print $3; }' | sort -u | tail -n 1)"
curl -O -L ${RC_URL}/"${ARCHIVE}"
SUBDIR="$(tar -tvf "${ARCHIVE}" | head -n 1 | rev | cut -d ' ' -f 1 | rev)"
tar -xvf "${ARCHIVE}"
cd "${SUBDIR}" || exit 1
./configure --prefix=/usr/local
make
make install
cd "${CWD}" || exit 1
rm -rf "${ARCHIVE}" "${SUBDIR}"

# Update the clamav database
freshclam
