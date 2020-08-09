#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# Install Linux kmod from source, we need the library
# NOTE: This is a port of kmod to build and run on MacOS X
cd ${CWD}
git clone https://github.com/dcantrell/kmod.git
cd kmod
./autogen.sh
./configure --prefix=/usr/local --with-xz --with-zlib --without-openssl
make

# The mandoc package from Homebrew in MacOS X lacks libmandoc.a and
# header files, which we need to build rpminspect
#
# NOTE: install(1) on MacOS X uses '-d' instead of the '-D' GNU
# spelling, but we don't actually need it for mandoc because the
# Makefile creates the directories before running install(1) and with
# BSD install if the directories already exist and you pass -d, it
# errors out.
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
echo 'INSTALL_PROGRAM="install -m 0755"' >> configure.local
echo 'INSTALL_LIB="install -m 0644"'     >> configure.local
echo 'INSTALL_HDR="install -m 0644"'     >> configure.local
echo 'INSTALL_MAN="install -m 0644"'     >> configure.local
echo 'INSTALL_DATA="install -m 0644"'    >> configure.local
echo 'INSTALL_LIBMANDOC=1'                  >> configure.local
echo 'CFLAGS="-g -fPIC"'                    >> configure.local
./configure
make
make lib-install
cd ..
rm -rf mandoc.tar.gz ${SUBDIR}

# Work around a bug in meson 0.55.0
MESON_VER="$(meson --version 2>/dev/null)"

if [ "${MESON_VER}" = "0.55.0" ]; then
    PYDIR="$(dirname $(find /usr/local/Cellar/meson/${MESON_VER} -name "ninjabackend.py"))"
    [ -d "${PYDIR}" ] || exit 1
    cd "${PYDIR}"
    [ -f "${OSDEPS}/meson.patch" ] || exit 2
    patch -p0 < "${OSDEPS}/meson.patch"
fi
