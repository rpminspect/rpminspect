#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

if [ "$(uname -m)" = "x86_64" ]; then
    dpkg --add-architecture i386
fi

apt-get update

# install and update pip and setuptools
apt-get install python3-pip python3-setuptools
pip3 install --upgrade pip setuptools
