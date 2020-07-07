#!/bin/sh
#
# Install required packages on Ubuntu to build rpminspect
#

PATH=/bin:/usr/bin
CWD="$(pwd)"

PKGS="gcc
meson
gettext
pkg-config
libjson-c-dev
libxml2-dev
libzstd-dev
librpm-dev
libarchive-dev
libelf-dev
libkmod-dev
libcurl4-openssl-dev
libyaml-dev
libssl-dev
libcap-dev
libcunit1-dev
ksh
rc
tcsh
zsh
valgrind
gcovr
python3-pip
libffi-dev
rpm
desktop-file-utils
sssd
python3-rpm
gcc-multilib"

# Install packages
sudo apt-get -y install ${PKGS}

# Install the kernel headers for the test suite
sudo apt-get -y install linux-headers-$(uname -r)

# Install 32-bit glibc for some of the tests
dpkg --add-architecture i386
apt-get update
sudo apt-get -y install libc-dev:i386

# Install things that use pip to install
sudo pip3 install cpp-coveralls rpmfluff PyYAML
