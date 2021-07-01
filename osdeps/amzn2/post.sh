#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

# Rebuild and install meson because of RPM dependency problems
cd ${HOME}
rpmdev-setuptree
yumdownloader --source meson
rpm -Uvh meson*.src.rpm
cd ${HOME}/rpmbuild/SPECS
rpmbuild -ba --define 'rpmmacrodir /usr/lib/rpm/macros.d' meson.spec
cd ${HOME}/rpmbuild/RPMS/noarch
rpm -Uvh meson*.noarch.rpm

# ninja symlink so we don't have to munge our Makefile
ln -sf ninja-build /usr/bin/ninja

# Update clamav database
freshclam
