#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# Install rust.  Slackware comes with Rust, but it's too old to build
# clamav, which we also need.
curl --proto '=https' --tlsv1.2 -sSf -o rustup.sh https://sh.rustup.rs
chmod +x rustup.sh
./rustup.sh -y
. ${HOME}/.cargo/env

# There is no clamav package in Slackware Linux
git clone -q https://github.com/Cisco-Talos/clamav.git
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
ninja
ninja install
cd "${CWD}" || exit 1

# There is no CUnit package, so build from source
svn checkout -q https://svn.code.sf.net/p/cunit/code/trunk cunit-code
cd cunit-code || exit 1
./bootstrap
./configure --prefix=/usr --libdir=/usr/lib64
make
make install
cd "${CWD}" || exit 1

# There is no xmlrpc-c package, so build from source
svn checkout -q https://svn.code.sf.net/p/xmlrpc-c/code/trunk xmlrpc-c-code
cd xmlrpc-c-code || exit 1
svn switch ^/stable
./configure --prefix=/usr --libdir=/usr/lib64
make
make install
cd "${CWD}" || exit 1

# There is no mandoc package, so build from source
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
( cd "${SUBDIR}" && ./configure && make && make lib-install )

# The 'rc' shell is not available in Slackware Linux, build manually
git clone -q https://github.com/rakitzis/rc.git
cd rc || exit 1
autoreconf -f -i -v
./configure --prefix=/usr
make
make install
cd "${CWD}" || exit 1

# Install libabigail from git
git clone -q git://sourceware.org/git/libabigail.git
cd libabigail || exit 1
TAG="$(git tag -l | grep ^libabigail- | grep -v '\.rc' | sort -n | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
autoreconf -f -i -v
./configure --prefix=/usr
make V=1
make install
cd "${CWD}" || exit 1

# Install annobin from source
git clone -q git://sourceware.org/git/annobin.git
cd annobin || exit 1
autoreconf -f -i -v
./configure --prefix=/usr
make
make install
cd "${CWD}" || exit 1

# cdson is not [yet] in Slackware Linux
git clone https://github.com/frozencemetery/cdson.git
cd cdson || exit 1
TAG="$(git tag -l | sort -n | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
meson setup build -D prefix=/usr
ninja -C build -v
ninja -C build test
ninja -C build install
cd "${CWD}" || exit 1
rm -rf cdson

# freshclam
cp /etc/clamav/freshclam.conf.sample /etc/clamav/freshclam.conf
sed -i -e '/^Example$/d' /etc/clamav/freshclam.conf
useradd clamav
mkdir -p /var/lib/clamav
chown 1000:1000 /var/lib/clamav
freshclam

# Update shared library cache
ldconfig
