#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# The mandoc package in Gentoo Linux is both masked and does not
# install the library.
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

# Makefile fixes for more or less Linux in general
sed -i -e 's|@echo|@/bin/echo|g' configure

./configure
make
make lib-install
cd ${CWD}
rm -rf mandoc.tar.gz ${SUBDIR}

# The 'rc' shell in Gentoo Linux is not the rc shell rpminspect
# expects, so just build it manually.
git clone https://github.com/rakitzis/rc.git
cd rc
autoreconf -f -i -v
./configure --prefix=/usr/local
make
make install
cd ${CWD}
rm -rf rc

# Update pip and setuptools
pip3 install --user --upgrade pip setuptools
( cd /root/.local/bin
  for f in * ; do
      ln -sf $(realpath ${f}) /usr/local/bin/${f}
  done
)

# Make sure the Linux source tree is usable for building modules
make -C /usr/src/linux olddefconfig
make -C /usr/src/linux prepare

# Update the clamav database
freshclam
