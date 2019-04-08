#!/bin/sh

PATH=/usr/bin
TOPDIR="$1"

# None of us is as dumb as all of us.
yum --help >/dev/null 2>&1
[ $? -eq 0 ] && INSTALLATOR=yum || INSTALLATOR=dnf

# Things required to build the source directly from git
${INSTALLATOR} install -y automake autoconf libtool

# Install the BuildRequires from the spec file
${INSTALLATOR} install -y $(grep BuildRequires: "${TOPDIR}/rpminspect.spec.in" | awk '{ print $2; }')
