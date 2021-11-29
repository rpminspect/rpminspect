#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# Install 32-bit development files on x86_64 systems
if [ "$(uname -m)" = "x86_64" ]; then
    yum install -y glibc-devel.i686 glibc.i686 libgcc.i686
fi

# We must install a more recent Python 3 (but exclude 3.10.x)
cd "${CWD}"
git clone https://github.com/python/cpython.git
cd cpython
TAG="$(git tag -l | grep -E '^v[0-9\.]+$' | grep -v 'v3\.10\.' | sort -V | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
./configure
make -j $(nproc)
make altinstall
PYTHON_VER="$(./python -c 'import sys ; print("%d.%d" % (sys.version_info[0], sys.version_info[1]))')"

# Now rebuild rpm because we need the Python bindings to use the newer
# Python.
cd "${CWD}"
yumdownloader --source python3-rpm
SRPM="$(ls -1 "${CWD}"/*.rpm)"
rpmdev-setuptree
rpm -Uvh "${SRPM}"
cd ~/rpmbuild/SPECS
sed -i -e 's|^Name:.*$|Name: python3-rpm-rebuild|g' python3-rpm.spec
yum install -y $(rpmspec -q --buildrequires python3-rpm.spec)
rpmbuild -ba --define "__python3 /usr/local/bin/python${PYTHON_VER}" python3-rpm.spec
rpm -Uvh ~/rpmbuild/RPMS/$(uname -m)/*.rpm

# Install Python modules for our recent Python.  Have to do this here
# because post.sh installed the Python we have to use for these tests.
pip${PYTHON_VER} install --upgrade pip setuptools
pip${PYTHON_VER} install cpp-coveralls gcovr PyYAML timeout-decorator rpmfluff

# Install the latest mandoc package to /usr/local, the official EPEL-7
# repos may be dated.
cd "${CWD}"
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

# Update clamav database
freshclam
