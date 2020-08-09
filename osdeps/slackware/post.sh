#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# The mandoc package on SlackBuilds.org lacks libmandoc.a and header
# files, which we need to build rpminspect
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
./configure
make
make lib-install
cd ${CWD}
rm -rf mandoc.tar.gz ${SUBDIR}

# The rpm build in Slackware only comes with the Python 2 module.  We
# need Python 3 support, so build and install rpm manually.
#
# Install to /usr because Python 3 on Slackware does not look in
# /usr/local by default.
git clone https://github.com/rpm-software-management/rpm.git
cd rpm
TAG="$(git tag -l | grep 'release$' | sort -V | tail -n 1)"
git checkout -b ${TAG} ${TAG}
autoreconf -i
CFLAGS="$(pkg-config --cflags nspr nss)" ./configure --prefix=/usr --without-lua --enable-python --without-selinux
make
make install
( cd python ; python3 setup.py install )
cd ${CWD}
rm -rf rpm
