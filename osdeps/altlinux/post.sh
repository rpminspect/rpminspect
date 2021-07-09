#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

# Install 32-bit development files on 64-bit systems when available
case "$(uname -m)" in
    x86_64)
        apt-get -y install i586-glibc-devel i586-libgcc1
        ;;
esac

# Alt Linux lacks mandoc
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
./configure
make
make lib-install
cd ..
rm -rf mandoc.tar.gz ${SUBDIR}

# The 'rc' shell is not available in Alt Linux, build manually
git clone https://github.com/rakitzis/rc.git
cd rc
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

# Create a test user to perform the build and run the test suite
# Alt Linux has patched rpmbuild to prevent running it as root.
useradd -c "Test User" -d /home/tester -m -s /bin/zsh tester

# Because Alt Linux lacks su, we have to use sudo and that requires
# rules files.  So add a sudo rules file to allow root to do things.
echo "root ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/root-nopasswd

# Update the clamav database
freshclam
