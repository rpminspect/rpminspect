#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# The mandoc package on OpenSUSE lacks libmandoc.a and
# header files, which we need to build rpminspect
if curl -s http://mandoc.bsd.lv/ >&- 2>&- ; then
    curl -O http://mandoc.bsd.lv/snapshots/mandoc.tar.gz
else
    # failed to connect to upstream host; take Debian's source
    DEBIAN_URL=http://ftp.debian.org/debian/pool/main/m/mdocml/
    # figure out which one is the latest and get that
    SRCFILE="$(curl -s ${DEBIAN_URL} 2>&- | sed -r 's/<[^>]*>//g' | sed -r 's/<[^>]*>$//g' | tr -s ' ' | grep -vE '^[ \t]*$' | grep ".orig.tar" | sed -r 's/[0-9]{4}-[0-9]{2}-[0-9]{2}.*$//g' | sort -n | tail -n 1)"
    curl -o mandoc.tar.gz ${DEBIAN_URL}/"${SRCFILE}"
fi
SUBDIR="$(tar -tvf mandoc.tar.gz | head -n 1 | awk '{ print $6; }')"
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

# unusual workarounds for executable on OpenSUSE Tumbleweed
sed -i -e 's|@echo|@/bin/echo|g' "${SUBDIR}"/configure

( cd "${SUBDIR}" && ./configure && make && make lib-install )
rm -rf mandoc.tar.gz "${SUBDIR}"

# The 'rc' shell is not available in OpenSUSE Tumbleweed, build manually
git clone -q https://github.com/rakitzis/rc.git
cd rc || exit 1
make
make install
cd "${CWD}" || exit 1
rm -rf rc

# Install libtoml from git
cd "${CWD}" || exit 1
git clone -q https://github.com/ajwans/libtoml.git
cd libtoml || exit 1
cmake .
make
make test
install -D -m 0755 libtoml.so /usr/lib64/libtoml.so
install -D -m 0644 toml.h /usr/include/toml.h
cd "${CWD}" || exit 1
rm -rf libtoml
ldconfig

# Update clamav database
freshclam
