#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# The mandoc package on OpenSUSE lacks libmandoc.a and
# header files, which we need to build rpminspect
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

# unusual workarounds for executable on OpenSUSE Tumbleweed
sed -i -e 's|@echo|@/bin/echo|g' configure
sed -i -e 's|^int dummy;$|extern int dummy;|g' compat_getline.c
sed -i -e 's|^int dummy;$|extern int dummy;|g' compat_err.c

./configure
make
make lib-install
cd ${CWD}
rm -rf mandoc.tar.gz ${SUBDIR}

# Download 'rc' from Fedora because OpenSUSE does not provide it
cat << "EOF" > /etc/yum/repos.d/fedora.repo
[fedora]
name=Fedora
metalink=https://mirrors.fedoraproject.org/metalink?repo=fedora-rawhide&arch=$basearch
enabled=1
metadata_expire=7d
rep_gpgcheck=0
type=rpm
gpgcheck=0
EOF

mkdir -p ${CWD}/fedora-rpms
yum install -y --downloadonly --downloaddir=${CWD}/fedora-rpms rc
cd ${CWD}/fedora-rpms
for pkg in *.rpm ; do
    rpm2cpio ${pkg} | cpio -id
done
cp -av ${CWD}/fedora-rpms/usr/* /usr/local
cd ${CWD}
rm -rf fedora-rpms
ldconfig

# Update clamav database
freshclam
