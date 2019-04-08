#!/bin/sh

PATH=/usr/bin

# None of us is as dumb as all of us.
yum --help >/dev/null 2>&1
[ $? -eq 0 ] && INSTALLATOR=yum || INSTALLATOR=dnf

# Things required to build the source directly from git
${INSTALLATOR} install -y automake autoconf libtool
