#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin
CWD="$(pwd)"

# Slackware Linux does not define an RPM dist tag
echo '%dist .ri47' > "${HOME}"/.rpmmacros

# Install a more recent Python than what comes with Slackware
cd "${CWD}" || exit 1
git clone -q https://github.com/python/cpython.git
cd cpython || exit 1
TAG="$(git tag -l | grep -E '^v[0-9\.]+$' | sort -V | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
./configure --prefix=/usr
make -j $(($(nproc) + 1))
make install
PYTHON_VER="$(./python -c 'import sys ; print("%d.%d" % (sys.version_info[0], sys.version_info[1]))')"
ln -s ../lib/python${PYTHON_VER} /usr/lib64/python${PYTHON_VER}
cd "${CWD}" || exit 1
rm -rf cpython

# Lua is required by RPM
cd "${CWD}" || exit 1
VER="$(git ls-remote -t https://github.com/lua/lua.git | grep -E "v[0-9\.]+$" | awk '{ print $2; }' | sort -V | tail -n 1 | rev | cut -d '/' -f 1 | rev | sed -e 's/v//g')"
curl -O https://lua.org/ftp/lua-${VER}.tar.gz
tar -xf lua-${VER}.tar.gz
cd lua-${VER} || exit 1
sed -i -e 's|lib/lua|lib64/lua|g' src/luaconf.h
make linux MYCFLAGS="-fPIC" INSTALL_TOP=/usr INSTALL_LIB=/usr/lib64
make linux install MYCFLAGS="-fPIC" INSTALL_TOP=/usr INSTALL_LIB=/usr/lib64
cd "${CWD}" || exit 1
rm -rf lua lua-${VER}.tar.gz

# Build rpm because it has Python bindings
cd "${CWD}" || exit 1
git clone -q https://github.com/rpm-software-management/rpm.git
cd rpm || exit 1
TAG="$(git tag -l | grep -E '^rpm-[0-9\.]+-release$' | sort -V | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
mkdir cmake-build
cd cmake-build || exit 1
cmake -D CMAKE_INSTALL_PREFIX=/usr \
      -D CMAKE_INSTALL_MANDIR=man \
      -D WITH_AUDIT=OFF \
      -D WITH_SELINUX=OFF \
      -D WITH_INTERNAL_OPENPGP=ON \
      -D ENABLE_TESTSUITE=OFF \
      ..
make -j $(($(nproc) + 1))
make install
cd "${CWD}" || exit 1
rm -rf rpm

# Update pip
pip${PYTHON_VER} install --upgrade pip

# Install additional Python modules
pip${PYTHON_VER} install --upgrade cpp-coveralls rpmfluff PyYAML gcovr timeout-decorator setuptools

# Install meson (can't use Slackware package of meson because it needs
# Slackware Python.)
cd "${CWD}" || exit 1
git clone -q https://github.com/mesonbuild/meson.git
cd meson || exit 1
TAG="$(git tag -l | grep -E '^[0-9\.]+$' | sort -V | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
python3 setup.py build
python3 setup.py install --prefix /usr
cd "${CWD}" || exit 1
rm -rf meson

# Install rust.  Slackware comes with Rust, but it's too old to build
# clamav, which we also need.
cd "${CWD}" || exit 1
curl --proto '=https' --tlsv1.2 -sSf -o rustup.sh https://sh.rustup.rs
chmod +x rustup.sh
./rustup.sh -y
# shellcheck disable=SC1091
. "${HOME}"/.cargo/env
rm -f rustup.sh

# There is no clamav package in Slackware Linux
cd "${CWD}" || exit 1
git clone -q https://github.com/Cisco-Talos/clamav.git
cd clamav || exit 1
TAG="$(git tag -l | grep -E "^clamav-[0-9\.]+$" | grep "\." | grep -v "\-rc" | sort -V | tail -n 1)"
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
ninja -j $(($(nproc) + 1))
ninja install
cd "${CWD}" || exit 1
rm -rf clamav

# There is no CUnit package, so build from source
cd "${CWD}" || exit 1
svn checkout -q https://svn.code.sf.net/p/cunit/code/trunk cunit-code
cd cunit-code || exit 1
./bootstrap
./configure --prefix=/usr --libdir=/usr/lib64
make -j $(($(nproc) + 1))
make install
cd "${CWD}" || exit 1
rm -rf cunit-code

# There is no xmlrpc-c package, so build from source
cd "${CWD}" || exit 1
svn checkout -q https://svn.code.sf.net/p/xmlrpc-c/code/trunk xmlrpc-c-code
cd xmlrpc-c-code || exit 1
svn switch ^/stable
./configure --prefix=/usr --libdir=/usr/lib64
make -j $(($(nproc) + 1))
make install
cd "${CWD}" || exit 1
rm -rf xmlrpc-c-code

# There is no mandoc package, so build from source
cd "${CWD}" || exit 1
if curl -s http://mandoc.bsd.lv/ >&- 2>&- ; then
    curl -O http://mandoc.bsd.lv/snapshots/mandoc.tar.gz
else
    # failed to connect to upstream host; take Debian's source
    DEBIAN_URL=http://ftp.debian.org/debian/pool/main/m/mdocml/
    # figure out which one is the latest and get that
    SRCFILE="$(curl -s ${DEBIAN_URL} 2>&- | sed -r 's/<[^>]*>//g' | sed -r 's/<[^>]*>$//g' | tr -s ' ' | grep -vE '^[ \t]*$' | grep ".orig.tar" | sed -r 's/[0-9]{4}-[0-9]{2}-[0-9]{2}.*$//g' | sort -n | tail -n 1)"
    curl -o mandoc.tar.gz ${DEBIAN_URL}/"${SRCFILE}"
fi
SUBDIR="$(tar -tvf mandoc.tar.gz | head -n 1 | rev | cut -d ' ' -f 1 | rev)"
tar -xf mandoc.tar.gz
{ echo 'PREFIX=/usr';
  echo 'BINDIR=/usr/bin';
  echo 'SBINDIR=/usr/sbin';
  echo 'MANDIR=/usr/share/man';
  echo 'INCLUDEDIR=/usr/include';
  echo 'LIBDIR=/usr/lib';
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
( cd "${SUBDIR}" && ./configure && make -j $(($(nproc) + 1)) && make lib-install )
cd "${CWD}" || exit 1
rm -rf mandoc.tar.gz "${SUBDIR}"

# The 'rc' shell is not available in Slackware Linux, build manually
cd "${CWD}" || exit 1
git clone -q https://github.com/rakitzis/rc.git
cd rc || exit 1
make -j $(($(nproc) + 1))
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
./configure --prefix=/usr
make -j $(($(nproc) + 1)) V=1
make install
cd "${CWD}" || exit 1
rm -rf libabigail

# Install annobin from source
cd "${CWD}" || exit 1
git clone -q git://sourceware.org/git/annobin.git
cd annobin || exit 1
autoreconf -f -i -v
./configure --prefix=/usr --without-clang-plugin --without-llvm-plugin
make -j $(($(nproc) + 1))
make install
cd "${CWD}" || exit 1
rm -rf annobin

# cdson is not [yet] in Slackware Linux
cd "${CWD}" || exit 1
git clone -q https://github.com/frozencemetery/cdson.git
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
