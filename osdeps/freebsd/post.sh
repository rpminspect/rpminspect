#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin

# Hostname to make sure rpmbuild works (this is gross)
echo "$(ifconfig | grep "inet " | grep -v "inet 127" | awk '{ print $2; }') $(hostname)" >> /etc/hosts

# Install Python modules from ports, but we have to determine the
# package prefix based on the version of Python installed.
PKG_PREFIX="py$(python3 --version | cut -d ' ' -f 2 | cut -d '.' -f 1,2 | sed -e 's|\.||g')"
pkg install -y ${PKG_PREFIX}-pip ${PKG_PREFIX}-pyaml ${PKG_PREFIX}-timeout-decorator

# Now install modules with pip
pip install cpp-coveralls gcovr rpmfluff

# libmandoc is missing on FreeBSD
curl -O https://mandoc.bsd.lv/snapshots/mandoc.tar.gz
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
  echo 'INSTALL_PROGRAM="install -m 0755"';
  echo 'INSTALL_LIB="install -m 0644"';
  echo 'INSTALL_HDR="install -m 0644"';
  echo 'INSTALL_MAN="install -m 0644"';
  echo 'INSTALL_DATA="install -m 0644"';
  echo 'INSTALL_LIBMANDOC=1';
  echo 'CFLAGS="-g -fPIC"';
} > "${SUBDIR}"/configure.local

( cd "${SUBDIR}" && ./configure && gmake && gmake lib-install )
rm -rf mandoc.tar.gz "${SUBDIR}"

# The ksh93 package does not install a 'ksh' executable, so create a symlink
ln -sf ksh93 /usr/local/bin/ksh

# Update the clamav database
freshclam
