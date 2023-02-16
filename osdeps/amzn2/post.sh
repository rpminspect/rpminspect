#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

# ninja symlink so we don't have to munge our Makefile
ln -sf ninja-build /usr/bin/ninja

# Rebuild and install meson because of RPM dependency problems
cd "${HOME}" || exit 1
rpmdev-setuptree
yumdownloader --source meson
rpm -Uvh meson*.src.rpm
cd "${HOME}"/rpmbuild/SPECS || exit 1
rpmbuild -ba --define 'rpmmacrodir /usr/lib/rpm/macros.d' meson.spec
cd "${HOME}"/rpmbuild/RPMS/noarch || exit 1
rpm -Uvh meson*.noarch.rpm

# cdson is not [yet] in Amazon Linux
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

# Update clamav database
freshclam
