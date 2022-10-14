#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# There is no cbindgen package in Slackware Linux
cargo install cbindgen

# There is no clamav package in Slackware Linux
git clone https://github.com/Cisco-Talos/clamav.git
cd clamav || exit 1
TAG="$(git tag -l | grep -E "^clamav-[0-9\.]+$" | grep "\." | sort -V | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
mkdir build
cd build || exit 1
cmake -D ENABLE_MILTER=OFF \
      -D ENABLE_JSON_SHARED=ON \
      -D CMAKE_INSTALL_PREFIX=/usr \
      -D CMAKE_INSTALL_LIBDIR=lib64 \
      -D APP_CONFIG_DIRECTORY=/etc/clamav \
      -D DATABASE_DIRECTORY=/var/lib/clamav \
      -G Ninja ..
ninja -v
ninja -v install
cd "${CWD}" || exit 1

# There is no CUnit package, so build from source
svn checkout https://svn.code.sf.net/p/cunit/code/trunk cunit-code
cd cunit-code || exit 1
./bootstrap
./configure --prefix=/usr --libdir=/usr/lib64
make
make install
cd "${CWD}" || exit 1

# There is no xmlrpc-c package, so build from source
svn checkout https://svn.code.sf.net/p/xmlrpc-c/code/trunk xmlrpc-c-code
cd xmlrpc-c-code || exit 1
svn switch ^/stable
./configure --prefix=/usr --libdir=/usr/lib64
make
make install
cd "${CWD}" || exit 1

# There is no mandoc package, so build from source
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

# The 'rc' shell is not available in Slackware Linux, build manually
git clone https://github.com/rakitzis/rc.git
cd rc || exit 1
autoreconf -f -i -v
./configure --prefix=/usr
make
make install
cd "${CWD}" || exit 1

# Install libabigail from git
git clone git://sourceware.org/git/libabigail.git
cd libabigail || exit 1
TAG="$(git tag -l | grep ^libabigail- | grep -v '\.rc' | sort -n | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
autoreconf -f -i -v
./configure --prefix=/usr
make
make install
cd "${CWD}" || exit 1

# Install annobin from source
git clone git://sourceware.org/git/annobin.git
cd annobin || exit 1
autoreconf -f -i -v
./configure --prefix=/usr
make
make install
cd "${CWD}" || exit 1

# Update shared library cache
ldconfig
