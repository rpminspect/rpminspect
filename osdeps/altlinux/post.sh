#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

# Install 32-bit development files on 64-bit systems when available
if [ "$(uname -m)" = "x86_64" ]; then
    apt-get -y install i586-glibc-devel i586-libgcc1
fi

# Alt Linux lacks mandoc
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

# The 'rc' shell is not available in Alt Linux, build manually
git clone https://github.com/rakitzis/rc.git
cd rc || exit 1
autoreconf -f -i -v
./configure --prefix=/usr/local
make
make install
cd ..
rm -rf rc

# This brp script on Alt Linux will prevent the deliberately broken
# shellsyntax tests from running, so just remove the file and let our
# test suite do what it wants.
rm -f /usr/lib/rpm/shell.req

# Alt Linux is configured to set the RPMTAG_VENDOR to always be "ALT
# Linux Team", so remove that macro.
find /usr/lib/rpm -type f -name macros -exec sed -i -e '/^%vendor/d' {} \;

# Create a test user to perform the build and run the test suite
# Alt Linux has patched rpmbuild to prevent running it as root.
useradd -c "Test User" -d /home/tester -m -s /bin/zsh tester

# Because Alt Linux lacks su, we have to use sudo and that requires
# rules files.  So add a sudo rules file to allow root to do things.
echo "root ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/root-nopasswd

# Update the clamav database
freshclam
